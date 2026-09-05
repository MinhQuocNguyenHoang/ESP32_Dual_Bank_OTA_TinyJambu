# OTA Flash, Dual-Bank Layout, and Rollback

The OTA implementation is based on the official ESP-IDF OTA APIs. The firmware does not write raw flash addresses directly. Application image writes go through `esp_ota_begin()`, `esp_ota_write()`, `esp_ota_end()`, and `esp_ota_set_boot_partition()`.

## Partition Table

The project uses a custom partition table in `partitions.csv`.

```text
# Name,    Type, SubType, Offset,    Size
nvs,       data, nvs,     0x9000,    0x4000
otadata,   data, ota,     0xd000,    0x2000
phy_init,  data, phy,     0xf000,    0x1000
ota_0,     app,  ota_0,   0x10000,   0x1F0000
ota_1,     app,  ota_1,   0x200000,  0x1F0000
```

`ota_0` and `ota_1` are application slots. At runtime, ESP32 executes the firmware from one slot. During OTA, the new firmware is written to the inactive slot. After validation succeeds, the bootloader metadata is updated so the next reboot starts from the new slot.

The current full firmware binary is about 980 KB, while each OTA slot is `0x1F0000` bytes. If a future binary exceeds the OTA slot size, `ota_writer_begin()` rejects the update with `ESP_ERR_INVALID_SIZE`.

## `otadata`

The `otadata` partition stores boot selection metadata used by the ESP-IDF bootloader. When the firmware calls `esp_ota_set_boot_partition(update_partition)`, ESP-IDF updates `otadata`. On the next reset, the bootloader reads that metadata and selects the new application partition.

The PC host does not need to know the physical flash address of `ota_0` or `ota_1`. The host sends only the firmware bytes and metadata. ESP32 selects the inactive update partition with:

```c
esp_ota_get_next_update_partition(NULL)
```

If the current firmware is running from `ota_0`, the update partition is normally `ota_1`. If the current firmware is running from `ota_1`, the update partition is normally `ota_0`.

## OTA Writer

`ota_writer.c` is a narrow wrapper around ESP-IDF OTA APIs. Its runtime context stores:

```text
update_partition
handle
image_size
bytes_written
active
```

`ota_writer_begin()` receives the expected image size, finds the next update partition, checks the image size against the partition size, and calls `esp_ota_begin()`.

`ota_writer_write()` writes the next sequential firmware chunk with `esp_ota_write()`. It checks that the context is active, the data pointer is valid, and the write does not exceed the expected image size.

`ota_writer_finish()` verifies that the number of written bytes matches the expected image size, calls `esp_ota_end()`, and optionally calls `esp_ota_set_boot_partition()`.

`ota_writer_abort()` cancels an active OTA session with `esp_ota_abort()`. It is used when the host sends ABORT or when the controller detects an image-level CRC32 failure.

## OTA Controller

`ota_controller.c` owns the OTA state machine.

| State | Meaning |
| --- | --- |
| `IDLE` | No active OTA session. |
| `RECEIVING` | Firmware chunks are being received and written. |
| `VERIFYING` | END_OTA was received and the firmware image is being verified. |
| `SUCCESS` | Image verification succeeded and the new boot partition was selected. |
| `FAILED` | The current OTA session failed. |

On `START_OTA`, the controller reads `image_size` and `expected_crc32` from the payload, calls `ota_writer_begin()`, resets byte counters, initializes CRC32 streaming, and sets `expected_seq = seq + 1`.

On `DATA`, the controller checks the session state, sequence number, payload length, and remaining image space. Valid payload bytes are written to flash and fed into `crc_util_crc32_update()`.

On `END_OTA`, the controller verifies that the total received byte count matches the expected image size, finalizes CRC32, compares it against the host value, sets the NVS pending flag, finishes the writer, selects the boot partition, and reboots.

On `ABORT`, the controller aborts the writer if a session is active, clears session metadata, and returns to idle state if the abort succeeds.

## Rollback With ESP-IDF and NVS

Rollback is implemented using ESP-IDF image state plus project-level NVS bookkeeping.

Rollback is enabled in `sdkconfig`.

```text
CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y
CONFIG_APP_ROLLBACK_ENABLE=y
```

The project stores two keys in NVS namespace `ota_store`.

```text
pending  -> whether the new image is waiting for validation
boot_cnt -> number of boot attempts while pending
```

Current limit:

```text
NVS_STORE_MAX_OTA_BOOT_ATTEMPTS = 3
```

Before selecting the new boot partition, the controller calls `nvs_store_set_ota_pending()`. This writes `pending = 1` and resets `boot_cnt = 0`.

When the new firmware boots, `app_main()` calls `ota_boot_health_check()`. If the running image is pending validation, the boot counter is incremented. If the counter becomes greater than 3, the firmware calls:

```c
esp_ota_mark_app_invalid_rollback_and_reboot()
```

If the image boots and reaches the application validation point, `app_main()` calls `ota_confirm_running_app_if_pending()`, which eventually calls:

```c
esp_ota_mark_app_valid_cancel_rollback()
```

After successful validation, the NVS pending flag and boot counter are cleared.

## Failure Cases

If the image is larger than the inactive OTA slot, the writer rejects the update before flash writing starts.

If power is lost during the write phase, the boot partition has not been changed yet. ESP32 keeps booting the previous firmware.

If power is lost after the boot partition is changed but before the new firmware is confirmed valid, ESP-IDF rollback state and the NVS counter protect the device from staying permanently on a bad image.

If final CRC32 does not match, the controller aborts the session and does not call `esp_ota_set_boot_partition()`.

If the new firmware repeatedly boots but fails before validation, the boot counter eventually exceeds the allowed attempts and the rollback API reboots into the previous valid image.
