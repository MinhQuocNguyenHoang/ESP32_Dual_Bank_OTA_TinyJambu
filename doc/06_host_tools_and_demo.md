# Host Tools and OTA Demonstration

The PC side uses `tools/ota_uart_send.py` to send a firmware binary over UART. The script reads the `.bin` file, calculates the full-image CRC32, sends START_OTA, splits the firmware into DATA packets, sends END_OTA, and waits for ACK/NACK after every packet.

The Python sender requires `pyserial`.

## Build the Main Firmware

From the project root:

```bash
idf.py build
```

The OTA binary is generated at:

```text
build/glucose_monitor_espidf.bin
```

If the board is still running an old image without the OTA task, flash the OTA-capable firmware through the normal programming port first:

```bash
./tools/flash_initial.sh /dev/ttyUSB0
```

After the firmware boots, wait for:

```text
OTA task started on UART2 RX=26 TX=27
```

The host should start the OTA transfer only after this log appears.

## Recommended Two-UART Demo Setup

The cleanest demo setup uses two serial links.

```text
UART0 onboard:
  initial flash and ESP-IDF monitor logs

UART2 GPIO26/GPIO27:
  UART OTA firmware transfer from the Python host
```

Monitor terminal:

```bash
idf.py -p /dev/ttyUSB0 monitor
```

OTA transfer terminal:

```bash
./tools/ota_full.sh /dev/ttyUSB1
```

`/dev/ttyUSB1` is only an example. Use the actual port of the external USB-UART adapter connected to GPIO26/GPIO27.

## Finding the Correct Serial Port

On Linux:

```bash
ls /dev/ttyUSB* /dev/ttyACM*
```

A reliable method is to unplug the external USB-UART adapter, run the command, plug the adapter back in, and run the command again. The newly appearing device is the OTA adapter.

If `idf.py monitor` shows ESP32 logs on `/dev/ttyUSB0`, that port is normally the board UART0. The OTA code currently listens on UART2, so the OTA sender should use the USB-UART adapter wired to GPIO26/GPIO27.

## Low-Level Python Sender

`tools/ota_uart_send.py` exposes the raw protocol sender.

| Argument | Meaning |
| --- | --- |
| `--port` | Serial port used for OTA transfer. |
| `--file` | Firmware `.bin` path. |
| `--baud` | UART baud rate, default 115200. |
| `--chunk-size` | Firmware payload bytes per DATA packet, maximum 256. |
| `--timeout` | ACK/NACK timeout per packet. |
| `--retries` | Retry count per packet. |
| `--settle-delay` | Delay after opening the serial port before sending. |

Direct usage:

```bash
python3 tools/ota_uart_send.py \
  --port /dev/ttyUSB1 \
  --file build/glucose_monitor_espidf.bin \
  --baud 115200
```

## Short Demo Scripts

The repository also provides shell wrappers for the most common operations.

| Script | Purpose |
| --- | --- |
| `tools/flash_initial.sh` | Flash the firmware through ESP-IDF and open monitor. |
| `tools/build_ota_test.sh` | Build only the minimal OTA test firmware. |
| `tools/ota_test.sh` | Build the minimal OTA test firmware and send it through UART OTA. |
| `tools/ota_full.sh` | Send the already built main firmware through UART OTA. |

Initial flash:

```bash
./tools/flash_initial.sh /dev/ttyUSB0
```

Send the main firmware from `build/glucose_monitor_espidf.bin`:

```bash
./tools/ota_full.sh /dev/ttyUSB1
```

Send a specific binary:

```bash
./tools/ota_full.sh /dev/ttyUSB1 build/glucose_monitor_espidf.bin
```

Build and send the minimal OTA test firmware:

```bash
./tools/ota_test.sh /dev/ttyUSB1 ota-demo-v2
```

Build the minimal OTA test firmware without sending it:

```bash
./tools/build_ota_test.sh ota-demo-v2
```

`ota_test.sh` builds into `build_ota_test` with `APP_OTA_TEST_MODE=ON`. The test firmware skips the glucose monitor application and prints version and heartbeat logs.

## Expected Host Output

The Python sender prints firmware metadata before transfer:

```text
Firmware : build/glucose_monitor_espidf.bin
Size     : 979936 bytes
CRC32    : 0x6650F41F
Port     : /dev/ttyUSB1 @ 115200
```

After that, it prints a progress bar based on the number of firmware bytes sent.

## Expected ESP32 Logs

Successful START:

```text
START OTA: image_size=... expected_crc32=...
Running partition: ...
Writing OTA image to: ...
```

During writing:

```text
WRITING: .../... bytes (...%)
```

After END:

```text
VERIFY: calculating firmware CRC32
SUCCESS: OTA image verified and boot partition updated
REBOOT: restarting after successful OTA
```

After reboot into the new image:

```text
OTA image pending validation, boot attempt 1/3
OTA image confirmed valid
```

## Troubleshooting

Timeout at `seq=0` means the START packet was sent but no ACK/NACK came back. Check that the board is running OTA-capable firmware, the OTA task has started, the correct serial port is used, TX/RX are crossed correctly, GND is shared, and the USB-UART adapter uses 3.3 V logic.

Timeout after several DATA packets usually points to an unstable UART link, long wires, a weak power supply, or a reset during transfer. Try a shorter cable, a lower baud rate, a larger timeout, or a smaller chunk size.

NACK `CRC_ERROR` at packet level usually means UART data corruption or a mismatch between host and firmware CRC16 implementations.

NACK `LENGTH_ERROR` usually means an invalid payload length, chunk size above 256 bytes, START payload not exactly 8 bytes, or END payload not empty.

NACK `SEQ_ERROR` means the host and firmware no longer agree on the next sequence number.

An `Image too large` log means the binary is larger than the inactive OTA partition. Increase the partition size or reduce firmware size.

For a basic OTA proof, use the minimal OTA test firmware first. Once the UART OTA path is proven stable, transfer the full glucose monitor firmware.
