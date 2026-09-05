# Firmware Architecture

The firmware is organized as one ESP-IDF component under `main/`, with internal folders for application code, hardware drivers, OTA modules, cryptography headers, and the embedded model. `app_main()` in `main/main.c` is the system entry point.

At boot, the firmware initializes NVS, checks OTA rollback state, initializes the selected application mode, confirms a pending OTA image if the application reaches a valid point, creates the OTA task, and then enters the main runtime loop.

## Boot Flow

```text
app_main
  -> app_nvs_init
  -> ota_boot_health_check
  -> glucose_monitor_init or ota_test_app_init
  -> ota_confirm_running_app_if_pending
  -> xTaskCreate(ota_task)
  -> while true:
       glucose_monitor_process or ota_test_app_process
```

`app_nvs_init()` initializes the ESP-IDF NVS partition. If NVS reports no free pages or a version mismatch, the firmware erases NVS and initializes it again. This is a common recovery path during development when NVS layout changes.

`ota_boot_health_check()` checks both the project NVS state and ESP-IDF OTA image state. If the running image is pending validation, the boot counter is incremented. If the counter exceeds the configured limit, the firmware marks the current image invalid and reboots into rollback.

The normal application path calls `glucose_monitor_init()`. This initializes the network stack, event loop, buzzer, MAX30102, SSD1306, WiFi, and MQTT. This function may wait up to roughly 10 seconds for WiFi and 6 seconds for MQTT.

The OTA test build path calls `ota_test_app_init()` instead. This mode avoids sensor, display, WiFi, MQTT, and TinyJAMBU initialization. It boots quickly and prints version information for OTA validation.

After the selected application init completes, `ota_confirm_running_app_if_pending()` marks the running image as valid if it was pending validation. In the current design, reaching this point is treated as the basic application self-test boundary.

## Runtime Tasks

The firmware has one main application loop and one dedicated OTA task.

```text
Main loop
  -> application process function
  -> vTaskDelay(10 ms)

OTA task
  -> ota_controller_init
  -> loop:
       ota_controller_process_once
```

`glucose_monitor_process()` runs the measurement state machine in the full application. In test mode, `ota_test_app_process()` prints a periodic heartbeat instead.

`ota_controller_process_once()` waits for one UART packet using the configured timeout. Timeout is treated as normal idle behavior. A valid packet is dispatched to the OTA state machine and produces either ACK or NACK.

## Glucose Measurement State Machine

The normal glucose application uses `display_state_t` in `glucose_monitor.c`.

| State | Responsibility |
| --- | --- |
| `STATE_IDLE` | Waits for finger placement and renders the idle screen. |
| `STATE_STABILIZING` | Waits for a stable IR signal for 5 seconds. |
| `STATE_SAMPLING` | Records Red/IR PPG samples at 100 Hz for a 2 second window. |
| `STATE_PREDICT` | Extracts features, runs the Gradient Boosting model, encrypts the result, and renders glucose output. |
| `STATE_UPLOADING` | Publishes encrypted telemetry through MQTT and waits for a gateway ACK. |
| `STATE_WAIT_RELEASE` | Waits until the user removes the finger before allowing another measurement. |

Main parameters:

| Parameter | Current value | Meaning |
| --- | ---: | --- |
| `FINGER_THRESHOLD` | 30000 | IR threshold used for finger detection. |
| `SAMPLE_WINDOW` | 2000 ms | PPG sampling duration for one measurement. |
| `SAMPLE_RATE` | 100 Hz | Logical sampling rate used by firmware. |
| `BUFFER_SIZE` | 200 samples | Number of Red/IR samples used for feature extraction. |
| `STABILIZE_WINDOW_MS` | 5000 ms | Stabilization time before sampling. |

## OTA Architecture

The OTA path is split into four modules.

| Module | Responsibility |
| --- | --- |
| `crc_until` | Provides CRC16 and CRC32 algorithms. |
| `uart_proto` | Configures UART, receives frames, validates CRC16, and sends ACK/NACK responses. |
| `ota_controller` | Owns START/DATA/END/ABORT state transitions, sequence checks, and image CRC32 verification. |
| `ota_writer` | Uses ESP-IDF OTA APIs to write firmware to the inactive app partition. |

Typical call flow for a valid packet:

```text
Python host
  -> UART frame
  -> uart_proto_receive_packet
  -> crc_util_crc16_ccitt
  -> ota_controller_handle_packet
  -> ota_writer_begin/write/finish
  -> uart_proto_send_ack or uart_proto_send_nack
```

`ota_controller` does not write flash directly. `ota_writer` does not understand the UART protocol. This keeps responsibilities narrow and satisfies the assignment constraint that layers must not share global state directly.

## Build Modes

Default mode builds the full glucose monitor application:

```bash
idf.py build
```

OTA test mode builds the minimal application:

```bash
idf.py -B build_ota_test \
  -DAPP_OTA_TEST_MODE=ON \
  -DAPP_OTA_TEST_VERSION=ota-demo-v2 \
  build
```

For day-to-day demonstration, the shell wrapper is shorter:

```bash
./tools/ota_test.sh /dev/ttyUSB1 ota-demo-v2
```
