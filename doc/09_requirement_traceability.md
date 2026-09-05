# Requirement Traceability

This file maps the assignment requirements from `doc/ota_doc.txt` to the current implementation. It is meant to support code review and demonstration by showing where each requirement is implemented.

## OTA Functional Requirements

| Requirement | Current implementation |
| --- | --- |
| Receive a binary firmware image from PC over UART | `tools/ota_uart_send.py` sends the `.bin`; ESP32 receives it through `uart_proto_receive_packet()` on UART2. |
| Use a custom packet-based UART protocol | Frame format is defined in `main/ota/uart_proto.h`; parsing is implemented in `main/ota/uart_proto.c`. |
| ACK/NACK after every packet | `uart_proto_send_ack()` and `uart_proto_send_nack()` are called after `ota_controller_process_once()` handles a packet. |
| Payload limited to 256 bytes per packet | `UART_PROTO_MAX_PAYLOAD_SIZE = 256U`; the Python sender also limits `MAX_PAYLOAD_SIZE` to 256. |
| START_OTA carries image size and CRC32 | `OTA_CONTROLLER_START_PAYLOAD_SIZE = 8U`; `ota_controller_handle_start()` reads image size and expected CRC32. |
| DATA writes firmware chunks | `ota_controller_handle_data()` calls `ota_writer_write()` for each DATA payload. |
| END_OTA verifies the complete image | `ota_controller_handle_end()` checks total byte count and compares CRC32. |
| ABORT cancels the session | `ota_controller_handle_abort()` calls `ota_writer_abort()` if a session is active. |

## CRC Requirements

| Requirement | Current implementation |
| --- | --- |
| CRC16 polynomial `0x1021` for every packet | `crc_util_crc16_ccitt()` uses polynomial `0x1021U`. |
| CRC16 covers header, sequence, length, and payload | `uart_proto_decode_frame()` calculates CRC16 over the frame without the final CRC field. |
| CRC32 verifies the complete firmware image | Python uses `zlib.crc32`; firmware uses `crc_util_crc32_init/update/finish`. |
| CRC32 is checked before boot partition switching | `ota_controller_handle_end()` calls `ota_writer_finish(..., true)` only after CRC32 matches. |

## ESP-IDF OTA API Requirements

| Requirement | Current implementation |
| --- | --- |
| Do not write flash directly | OTA flash writes are isolated in `ota_writer.c` and use ESP-IDF OTA APIs. |
| Use `esp_ota_begin()` | `ota_writer_begin()` calls `esp_ota_begin(update_partition, image_size, &ctx->handle)`. |
| Use `esp_ota_write()` | `ota_writer_write()` calls `esp_ota_write(ctx->handle, data, len)`. |
| Use `esp_ota_end()` | `ota_writer_finish()` calls `esp_ota_end(ctx->handle)`. |
| Use `esp_ota_set_boot_partition()` | `ota_writer_finish()` calls `esp_ota_set_boot_partition(update_partition)` when boot switching is requested. |

## Dual-Bank Flash Requirements

| Requirement | Current implementation |
| --- | --- |
| Two OTA application slots | `partitions.csv` defines `ota_0` and `ota_1`, each with size `0x1F0000`. |
| New firmware is written to the inactive slot | `ota_writer_begin()` uses `esp_ota_get_next_update_partition(NULL)`. |
| Bootloader starts the new image after verification | `esp_ota_set_boot_partition()` updates `otadata`, then firmware calls `esp_restart()`. |

## Rollback Requirements

| Requirement | Current implementation |
| --- | --- |
| Enable ESP-IDF rollback | `sdkconfig` contains `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y` and `CONFIG_APP_ROLLBACK_ENABLE=y`. |
| Use NVS for update attempt bookkeeping | `nvs_store.c` stores `pending` and `boot_cnt` under namespace `ota_store`. |
| Roll back after more than 3 resets without validation | `ota_boot_health_check()` increments the boot counter and calls rollback when the counter is greater than `NVS_STORE_MAX_OTA_BOOT_ATTEMPTS`. |
| Confirm the new firmware as valid | `ota_confirm_running_app_if_pending()` calls `ota_writer_mark_running_valid()` and clears the NVS pending state. |

## UART Driver Requirements

| Requirement | Current implementation |
| --- | --- |
| Interrupt-driven UART receive with FreeRTOS queue | `uart_driver_install()` creates `s_uart_event_queue`; receive logic uses `xQueueReceive()`. |
| Do not poll UART hardware status registers | The code uses ESP-IDF UART driver APIs and does not read UART registers directly. |
| Packet receive timeout | `ota_config.packet_timeout_ticks = pdMS_TO_TICKS(OTA_PACKET_TIMEOUT_MS)`. |

## Python Host Script Requirements

| Requirement | Current implementation |
| --- | --- |
| Read firmware binary | `Path(args.file).read_bytes()` in `tools/ota_uart_send.py`. |
| Split payload into packets | `for offset in range(0, len(image), args.chunk_size)`. |
| Retry each packet up to 3 times | `--retries` defaults to 3; `send_frame_with_retry()` retries on timeout or NACK. |
| Display progress | `print_progress(sent, len(image))`. |
| Calculate image CRC32 | `zlib.crc32(image) & 0xFFFFFFFF`. |
| Calculate packet CRC16 | `crc16_ccitt()` in the Python sender. |
| Provide convenient demo scripts | `tools/ota_test.sh`, `tools/ota_full.sh`, `tools/build_ota_test.sh`, and `tools/flash_initial.sh` wrap the long commands. |

## Status Logging Requirements

| Stage | Current log |
| --- | --- |
| START | `START OTA: image_size=... expected_crc32=...` |
| WRITING | `WRITING: .../... bytes (...%)` |
| VERIFY | `VERIFY: calculating firmware CRC32` |
| SUCCESS | `SUCCESS: OTA image verified and boot partition updated` |
| FAIL | Error branches in `ota_controller.c` and `ota_writer.c` use `ESP_LOGE`. |
| REBOOT | `REBOOT: restarting after successful OTA` and `Restarting to boot selected OTA partition`. |

## Notes for Demonstration

The Python host does not write to a fixed flash address. It sends the firmware binary and metadata. ESP32 chooses the inactive OTA slot through the ESP-IDF OTA API. The actual flash address is therefore the address of either `ota_0` or `ota_1`, depending on the currently running slot.

UART OTA is not ESP32 bootloader flashing. The board does not need to be placed in BOOT mode. The application firmware must be running and the OTA task must be initialized.

The first OTA-capable firmware still has to be flashed with `idf.py flash`. After that, future firmware images can be transferred through UART OTA.

When UART2 is used for OTA and UART0 is used for logs, the clean demo setup uses two serial links. This prevents binary OTA packets from mixing with human-readable logs.

The minimal OTA test firmware is implemented in `main/app/ota_test_app.c`. It does not replace the main firmware; it is a separate build mode used to prove version changes and boot partition switching.
