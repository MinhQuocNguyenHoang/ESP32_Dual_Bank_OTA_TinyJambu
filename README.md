# Non-Invasive Glucose Monitoring

This repository contains an ESP-IDF firmware project for a non-invasive glucose monitoring prototype based on ESP32, MAX30102, SSD1306 OLED, TinyJAMBU, MQTT, and dual-bank UART OTA.

The main technical documentation is in [doc](doc/README.md). Start with [doc/README.md](doc/README.md), then read the numbered files in order.

Build the main firmware:

```bash
idf.py build
```

The OTA firmware binary is generated at:

```text
build/glucose_monitor_espidf.bin
```

The current OTA transport uses UART2. ESP32 GPIO26 is the OTA RX pin and GPIO27 is the OTA TX pin. The first firmware image that enables OTA must still be flashed through the normal programming port. After that, newer images can be transferred with the project UART OTA tools.

Initial flash with monitor:

```bash
./tools/flash_initial.sh /dev/ttyUSB0
```

Send the already built main firmware through UART OTA:

```bash
./tools/ota_full.sh /dev/ttyUSB1
```

Build and send the minimal OTA test firmware:

```bash
./tools/ota_test.sh /dev/ttyUSB1 ota-demo-v2
```

The OTA test firmware prints its version, running partition, and heartbeat logs. It is intended for quickly verifying that OTA has rebooted into the new image.
