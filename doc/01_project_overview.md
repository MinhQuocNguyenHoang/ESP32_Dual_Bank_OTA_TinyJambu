# Project Overview

The project implements a non-invasive glucose monitoring prototype using ESP32, a MAX30102 optical sensor, an SSD1306 OLED display, an embedded machine learning model, encrypted telemetry, MQTT communication, and a custom UART OTA update path.

The firmware is written in C on top of ESP-IDF. It combines signal acquisition, simple PPG feature extraction, model inference, TinyJAMBU-128 AEAD encryption, local display output, MQTT upload, and a dual-bank firmware update system. The OTA path receives a firmware binary from a PC host over UART, validates each packet with CRC16, validates the full image with CRC32, writes the image to the inactive OTA partition, switches the next boot partition, and restarts the device.

## Main Capabilities

The device reads Red and IR samples from the MAX30102 sensor over I2C. When a finger is detected, the firmware waits for the signal to stabilize, records a fixed PPG sample window, extracts features, runs the exported Gradient Boosting model, and displays the predicted glucose value on the OLED.

The predicted value is encoded as a short plaintext string, encrypted with TinyJAMBU-128 AEAD, converted to a hex string, and published through MQTT. A Python dashboard subscribes to the telemetry topic, decrypts the message, stores measurements in SQLite, emits live updates through WebSocket, and sends an MQTT ACK back to the ESP32.

The OTA subsystem is packet based. The PC host sends START, DATA, END, and optional ABORT commands. ESP32 responds to every valid command path with ACK or NACK. This makes the transfer observable and recoverable at packet level.

## Source Tree

| Path | Responsibility |
| --- | --- |
| `main/main.c` | Firmware entry point, NVS initialization, rollback health check, application initialization, and OTA task creation. |
| `main/app/glucose_monitor.c` | Glucose measurement state machine, OLED UI, WiFi, MQTT, TinyJAMBU encryption, and telemetry upload. |
| `main/app/ota_test_app.c` | Minimal application used to validate UART OTA and confirm version changes after reboot. |
| `main/drivers/max30102.c` | Native ESP-IDF I2C driver for MAX30102 Red/IR FIFO sampling. |
| `main/drivers/ssd1306.c` | SSD1306 OLED display driver used by the glucose UI. |
| `main/model/gradient_boosting_model.h` | Exported Gradient Boosting model used by firmware inference. |
| `main/ota/crc_until.c` | CRC16/CCITT-FALSE and reflected CRC32 utilities. |
| `main/ota/uart_proto.c` | UART packet protocol layer: framing, parsing, CRC16 validation, ACK/NACK transmission. |
| `main/ota/ota_controller.c` | OTA state machine: START/DATA/END/ABORT handling, sequence checks, CRC32 streaming, and writer coordination. |
| `main/ota/ota_writer.c` | Wrapper around ESP-IDF OTA APIs for writing, finishing, selecting boot partition, aborting, and rollback operations. |
| `main/ota/nvs_store.c` | NVS storage for OTA pending state and boot attempt counter. |
| `tools/ota_uart_send.py` | Low-level Python sender for the custom UART OTA protocol. |
| `tools/ota_demo_flash.py` | Python wrapper that can build a firmware image and call `ota_uart_send.py`. |
| `tools/*.sh` | Short shell scripts for initial flashing, building OTA test firmware, sending OTA test firmware, and sending the main firmware. |
| `app/app.py` | Python Flask dashboard that receives MQTT telemetry, decrypts TinyJAMBU payloads, stores SQLite history, and sends ACKs. |
| `app/Gateway.py` | MQTT bridge from the local broker to ThingsBoard. |
| `software/train_model.py` | Model training script for sample glucose data. |
| `partitions.csv` | Custom partition table containing NVS, OTA data, PHY init, `ota_0`, and `ota_1`. |

## Layer Boundaries

The OTA implementation is intentionally split into small layers. `uart_proto` owns only UART framing and packet-level CRC16 validation. `ota_controller` owns the OTA state machine and decides whether a packet is valid for the current session. `ota_writer` owns flash write operations through ESP-IDF OTA APIs. `nvs_store` owns persistent rollback bookkeeping. `crc_until` is a standalone checksum utility module and does not know about UART, flash, or OTA state.

This separation keeps the UART transport independent from flash writing. It also allows the protocol layer to be replaced later by another transport, such as BLE, SPI, or TCP, while keeping most of the controller and writer logic intact.

## Hardware Mapping

| Function | Interface | ESP32 pins |
| --- | --- | --- |
| MAX30102 | I2C0 | SDA GPIO32, SCL GPIO33 |
| SSD1306 OLED | I2C0 | SDA GPIO32, SCL GPIO33 |
| Buzzer | LEDC PWM | GPIO25 |
| OTA UART | UART2 | RX GPIO26, TX GPIO27 |
| Default log/flash | UART0 | Board-dependent onboard USB-UART |

External USB-UART wiring for OTA:

```text
USB-UART TX -> ESP32 GPIO26
USB-UART RX -> ESP32 GPIO27
USB-UART GND -> ESP32 GND
```

UART logic level must be 3.3 V. ESP32 GPIO pins should not receive 5 V UART signals.

## Current Project State

The project currently supports end-to-end UART OTA. A firmware image can be sent from Python, written to the inactive OTA slot, verified with CRC32, selected as the next boot image, and booted after restart.

The first OTA-capable firmware still has to be flashed through the normal programming interface. After that, future builds can be transferred through UART OTA.

The OTA task is created after the main application initialization. The full glucose application can wait for WiFi and MQTT during startup, so a demo should wait for the log line `OTA task started on UART2 RX=26 TX=27` before sending the OTA image.

The project also provides a minimal OTA test build mode. When `APP_OTA_TEST_MODE` is enabled, the firmware skips the glucose pipeline and prints only version, running partition, and heartbeat logs. This mode is useful for quickly proving that OTA changed the running image.
