# UART OTA Protocol

The project uses a custom packet-based UART protocol between a PC host and the ESP32 firmware. The host reads a firmware `.bin` file, splits it into chunks of at most 256 payload bytes, sends the chunks through UART, and waits for an ACK or NACK after each packet.

This protocol is not the built-in Espressif ROM bootloader protocol. It runs inside the application firmware. The ESP32 must boot normally into an OTA-capable application; BOOT download mode is not used for UART OTA.

## UART Configuration

The current firmware configures the OTA transport in `main/main.c`.

```text
UART port: UART_NUM_2
Baud rate: 115200
RX buffer: 4096 bytes
TX buffer: 512 bytes
Event queue size: 20
ESP32 RX: GPIO26
ESP32 TX: GPIO27
```

External USB-UART wiring:

```text
USB-UART TX -> ESP32 GPIO26
USB-UART RX -> ESP32 GPIO27
USB-UART GND -> ESP32 GND
```

The UART adapter must use 3.3 V logic. If the ESP32 board is already powered through its main USB connector, the external adapter usually needs only TX, RX, and GND.

## Host-to-Device Frame

Every host-to-device frame has this format:

```text
[MAGIC0:1][MAGIC1:1][CMD:1][SEQ:2][LEN:2][PAYLOAD:0..256][CRC16:2]
```

Multi-byte fields are encoded little-endian.

| Field | Size | Meaning |
| --- | ---: | --- |
| `MAGIC0` | 1 byte | `0xA5` |
| `MAGIC1` | 1 byte | `0x5A` |
| `CMD` | 1 byte | OTA command identifier |
| `SEQ` | 2 bytes | Packet sequence number |
| `LEN` | 2 bytes | Payload length |
| `PAYLOAD` | 0 to 256 bytes | Command-specific payload |
| `CRC16` | 2 bytes | CRC16 over the frame without the CRC field |

CRC16 is calculated over:

```text
MAGIC0 || MAGIC1 || CMD || SEQ || LEN || PAYLOAD
```

The final two CRC bytes are not included in the CRC input.

## Commands

| Command | Value | Payload | Meaning |
| --- | ---: | --- | --- |
| `START_OTA` | `0x01` | 8 bytes | Starts a new OTA session. Payload contains image size and expected CRC32. |
| `DATA` | `0x02` | 1 to 256 bytes | Carries one firmware image chunk. |
| `END_OTA` | `0x03` | 0 bytes | Ends the transfer and asks ESP32 to verify the full image. |
| `ABORT` | `0x04` | 0 bytes | Cancels the current OTA session. |

`START_OTA` payload:

```text
[IMAGE_SIZE:4][EXPECTED_CRC32:4]
```

Both fields are little-endian. `IMAGE_SIZE` is the total firmware binary size in bytes. `EXPECTED_CRC32` is calculated by the host before sending any data packets.

## Sequence Numbers

The first packet uses `SEQ = 0`. Each accepted packet increments the expected sequence by one.

Normal transfer:

```text
START_OTA  seq=0
DATA       seq=1
DATA       seq=2
DATA       seq=3
...
END_OTA    seq=N
```

The ESP32 stores `expected_seq` in the OTA controller. If a DATA or END packet arrives with an unexpected sequence number, the controller rejects it with `SEQ_ERROR`. This protects the transfer from missing packets, duplicated packets, and out-of-order data.

## ACK/NACK Response

ESP32 sends a response frame after packet processing.

```text
[MAGIC0:1][MAGIC1:1][RESP:1][SEQ:2][LEN:2][STATUS:1][CRC16:2]
```

| Field | Value |
| --- | --- |
| `RESP_ACK` | `0x79` |
| `RESP_NACK` | `0x1F` |
| `LEN` | `0x0001` |
| `STATUS` | Processing status |

Status codes:

| Status | Value | Meaning |
| --- | ---: | --- |
| `OK` | `0x00` | Packet was accepted and processed. |
| `CRC_ERROR` | `0x01` | Packet CRC16 or final image CRC32 failed. |
| `SEQ_ERROR` | `0x02` | Sequence number does not match the expected value. |
| `LENGTH_ERROR` | `0x03` | Payload length is invalid or would exceed the expected image size. |
| `STATE_ERROR` | `0x04` | Command is not valid in the current OTA state. |
| `INTERNAL_ERROR` | `0x05` | Internal failure, usually from flash or driver APIs. |

The Python host sends the next packet only after receiving ACK with status OK for the expected sequence. Timeout or NACK triggers a retry. The default retry count is 3.

## Complete OTA Flow

```text
PC host                         ESP32
   |                              |
   | START_OTA(seq=0)             |
   |----------------------------->|
   |                              | validate CRC16
   |                              | read image_size and expected_crc32
   |                              | esp_ota_begin on inactive partition
   | ACK(seq=0)                   |
   |<-----------------------------|
   |                              |
   | DATA(seq=1, chunk 0)         |
   |----------------------------->|
   |                              | validate CRC16
   |                              | check sequence
   |                              | esp_ota_write
   |                              | crc32_update
   | ACK(seq=1)                   |
   |<-----------------------------|
   |                              |
   | DATA(seq=2..N-1)             |
   |----------------------------->|
   | ACK(seq=2..N-1)              |
   |<-----------------------------|
   |                              |
   | END_OTA(seq=N)               |
   |----------------------------->|
   |                              | verify total byte count
   |                              | crc32_finish
   |                              | compare expected_crc32
   |                              | esp_ota_end
   |                              | esp_ota_set_boot_partition
   | ACK(seq=N)                   |
   |<-----------------------------|
   |                              | reboot
```

## Common Demo Errors

Timeout at `seq=0` means the host sent START but did not receive ACK or NACK. Common causes are old firmware without the OTA task, wrong serial port, swapped TX/RX, missing common ground, 5 V UART logic, or sending before the OTA task has started.

If ESP32 logs `START OTA` but Python still times out, the host-to-ESP32 direction works and the problem is likely the ESP32 TX GPIO27 to USB-UART RX path.

`LENGTH_ERROR` usually means the payload length is invalid, the chunk size is larger than 256 bytes, START payload is not exactly 8 bytes, or END contains a payload.

`SEQ_ERROR` means host and firmware are out of sync. This can happen if the host restarts from packet zero while the ESP32 still has an active session.

`CRC_ERROR` on END means the transfer reached the end but the calculated firmware CRC32 does not match the expected CRC32 sent in START.
