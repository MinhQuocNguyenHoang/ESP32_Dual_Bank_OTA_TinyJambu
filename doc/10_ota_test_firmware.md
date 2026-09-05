# OTA Test Firmware and Demo Scripts

The project includes a minimal build mode for testing OTA independently from the glucose measurement pipeline. This mode builds a smaller firmware image that boots quickly and does not initialize WiFi, MQTT, sensors, OLED, or TinyJAMBU.

After a successful OTA update, the test firmware prints its version, running partition, and heartbeat logs. This makes it easy to confirm that ESP32 has rebooted into the new image.

## Purpose

The full firmware initializes MAX30102, SSD1306, WiFi, MQTT, TinyJAMBU, and the glucose measurement state machine. These subsystems are useful for the final project, but they can slow down OTA testing or introduce unrelated failures during a demo.

The OTA test firmware keeps only the pieces needed to validate the OTA mechanism:

```text
NVS initialization
rollback health check
OTA controller task
version log
heartbeat log
```

This verifies three important behaviors:

```text
the binary was received through UART
ESP32 wrote the image to the inactive OTA slot
the bootloader started the new image after reset
```

## Source Files

The test application source files are:

```text
main/app/ota_test_app.c
main/app/ota_test_app.h
```

Expected logs:

```text
Minimal OTA test firmware started
Version: ota-demo-v2
Running partition: ota_1 at 0x00200000
OTA test heartbeat=1, free_heap=... bytes, version=ota-demo-v2
```

The version string is passed from CMake through `APP_OTA_TEST_VERSION`. This allows each test build to carry a different version label without editing source code.

## Manual Test Firmware Build

Build the minimal OTA test firmware manually:

```bash
idf.py -B build_ota_test \
  -DAPP_OTA_TEST_MODE=ON \
  -DAPP_OTA_TEST_VERSION=ota-demo-v2 \
  build
```

The generated binary is:

```text
build_ota_test/glucose_monitor_espidf.bin
```

The ESP-IDF project name remains `glucose_monitor_espidf`; only the selected application source changes because `APP_OTA_TEST_MODE` is enabled.

## Short Shell Scripts

The recommended way to run demos is to use the shell scripts under `tools/`.

Build and send the minimal OTA test firmware:

```bash
./tools/ota_test.sh /dev/ttyUSB1 ota-demo-v2
```

This command builds `build_ota_test/glucose_monitor_espidf.bin` and sends it through UART OTA.

Build only, without sending:

```bash
./tools/build_ota_test.sh ota-demo-v2
```

Build with a custom build directory:

```bash
./tools/build_ota_test.sh ota-demo-v3 build_ota_v3
```

Send a specific binary:

```bash
./tools/ota_full.sh /dev/ttyUSB1 build_ota_v3/glucose_monitor_espidf.bin
```

Send the normal firmware from the default build directory:

```bash
./tools/ota_full.sh /dev/ttyUSB1
```

Initial flash through the normal programming port:

```bash
./tools/flash_initial.sh /dev/ttyUSB0
```

## Python Wrapper

The shell scripts call `tools/ota_demo_flash.py` internally. It can also be used directly.

Build and send OTA test firmware:

```bash
python3 tools/ota_demo_flash.py \
  --port /dev/ttyUSB1 \
  --test-app \
  --version ota-demo-v2
```

Send an existing binary:

```bash
python3 tools/ota_demo_flash.py \
  --port /dev/ttyUSB1 \
  --file build/glucose_monitor_espidf.bin
```

The lower-level sender remains available as:

```bash
python3 tools/ota_uart_send.py \
  --port /dev/ttyUSB1 \
  --file build/glucose_monitor_espidf.bin \
  --baud 115200
```

## Suggested Demo Flow

First, flash an OTA-capable firmware through the normal programming port:

```bash
./tools/flash_initial.sh /dev/ttyUSB0
```

Wait for this log:

```text
OTA task started on UART2 RX=26 TX=27
```

Then send the minimal test firmware from another terminal:

```bash
./tools/ota_test.sh /dev/ttyUSB1 ota-demo-v2
```

After the OTA transfer completes, ESP32 reboots. The monitor should show the new test firmware version. To return to the full glucose monitor firmware:

```bash
idf.py build
./tools/ota_full.sh /dev/ttyUSB1
```

## Environment Notes

`ota_test.sh`, `build_ota_test.sh`, and `flash_initial.sh` try to source ESP-IDF automatically from `$IDF_PATH/export.sh` or `$HOME/Tools/esp/esp-idf/export.sh`.

If `ota_demo_flash.py` is called directly and it needs to build firmware, the terminal must already have ESP-IDF in the environment:

```bash
source /home/minhquocnguyenhoang/Tools/esp/esp-idf/export.sh
```

If an existing `.bin` file is supplied, `idf.py` is not needed. `pyserial` is still required because the OTA transfer uses UART.

The test firmware still uses the same OTA task, NVS pending flag, and rollback health check as the main firmware. It is only a smaller application layer for repeatable OTA validation.
