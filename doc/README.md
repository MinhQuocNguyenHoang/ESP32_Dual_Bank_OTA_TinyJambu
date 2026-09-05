# Technical Documentation

This documentation describes the architecture, runtime flow, and operating procedure of the ESP32 non-invasive glucose monitoring project. The UART OTA subsystem follows the assignment requirements: custom UART firmware transfer, dual-bank flash, CRC16 per packet, CRC32 for the full image, official ESP-IDF OTA APIs, and rollback bookkeeping with NVS.

The documentation is split by technical area so each part can be read independently during development, review, or demonstration.

| File | Content |
| --- | --- |
| [01_project_overview.md](01_project_overview.md) | System overview, major features, source tree, and module responsibilities. |
| [02_firmware_architecture.md](02_firmware_architecture.md) | ESP32 firmware architecture, boot flow, tasks, state machines, and layer boundaries. |
| [03_uart_ota_protocol.md](03_uart_ota_protocol.md) | UART OTA packet format, command set, ACK/NACK handling, sequence numbers, and demo wiring. |
| [04_crc_integrity.md](04_crc_integrity.md) | CRC16/CCITT-FALSE for packets and CRC32 for full firmware image verification. |
| [05_ota_flash_and_rollback.md](05_ota_flash_and_rollback.md) | OTA writer, dual-bank partition table, boot partition switching, NVS pending state, boot counter, and rollback. |
| [06_host_tools_and_demo.md](06_host_tools_and_demo.md) | Build, initial flash, Python UART transfer, demo scripts, expected logs, and troubleshooting notes. |
| [07_glucose_pipeline.md](07_glucose_pipeline.md) | PPG acquisition from MAX30102, feature extraction, model inference, TinyJAMBU encryption, and MQTT telemetry. |
| [08_module_api_reference.md](08_module_api_reference.md) | Internal firmware API summary grouped by module. |
| [09_requirement_traceability.md](09_requirement_traceability.md) | Mapping between assignment requirements and the current implementation. |
| [10_ota_test_firmware.md](10_ota_test_firmware.md) | Minimal OTA test firmware and Python/shell scripts used for repeatable OTA demonstrations. |

The ESP-IDF project name is:

```text
glucose_monitor_espidf
```

The default OTA binary path after a normal build is:

```text
build/glucose_monitor_espidf.bin
```

Current UART OTA configuration:

```text
ESP32 UART2 RX: GPIO26
ESP32 UART2 TX: GPIO27
Baud rate: 115200
```

The default log and initial flashing port still use the board UART0. A clean demonstration should use two serial links: one for flashing and logs, and one external USB-UART adapter connected to GPIO26/GPIO27 for OTA transfer.
