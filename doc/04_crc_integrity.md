# CRC and Data Integrity

The project uses two CRC layers. CRC16/CCITT-FALSE protects each UART packet. CRC32 protects the complete firmware image after the transfer finishes. These checks are complementary because they catch errors at different points in the update flow.

CRC16 catches corruption early at packet level. If one packet is damaged, ESP32 can reject only that packet and the host can retry it. CRC32 is the final image-level check. It verifies that the complete binary received by ESP32 matches the file that the host intended to send.

## CRC16/CCITT-FALSE

Firmware API:

```c
uint16_t crc_util_crc16_ccitt(const uint8_t *data, uint32_t len);
```

Algorithm parameters:

| Property | Value |
| --- | --- |
| Common name | CRC-16/CCITT-FALSE |
| Polynomial | `0x1021` |
| Initial value | `0xFFFF` |
| Input reflection | Disabled |
| Output reflection | Disabled |
| Final XOR | None |
| Width | 16 bits |

For UART packets, CRC16 is calculated over the header and payload:

```text
[MAGIC0][MAGIC1][CMD][SEQ][LEN][PAYLOAD]
```

The two CRC bytes at the end of the frame are not included in the CRC input. The final CRC16 value is stored little-endian in the frame.

Example START frame CRC input:

```text
A5 5A 01 00 00 08 00 <image_size 4 bytes> <crc32 4 bytes>
```

Field meaning:

```text
A5 5A       magic
01          START_OTA command
00 00       sequence = 0
08 00       payload length = 8
```

The firmware recalculates CRC16 on the same byte range. If the received CRC16 does not match the calculated CRC16, the frame is rejected and the controller returns NACK with `CRC_ERROR`.

## CRC16 Calculation Steps

The implementation processes one byte at a time. Each byte is XORed into the high byte of the 16-bit CRC register:

```c
crc = crc ^ (data[i] << 8);
```

Then the algorithm loops 8 times, one step for each bit:

```c
if ((crc & 0x8000) != 0)
{
    crc = (crc << 1) ^ 0x1021;
}
else
{
    crc = crc << 1;
}
```

`0x8000` checks the top bit of the 16-bit CRC register. If the top bit is 1, the shifted value is XORed with polynomial `0x1021`. If the top bit is 0, the value is only shifted left. The result is stored in `uint16_t`, so only the lower 16 bits are kept.

XOR is not arithmetic addition. It is a bitwise operation:

```text
0 xor 0 = 0
0 xor 1 = 1
1 xor 0 = 1
1 xor 1 = 0
```

Example:

```text
0xFFFF xor 0xAA00 = 0x55FF
```

For the high byte:

```text
0xFF xor 0xAA = 0x55
1111 1111 xor 1010 1010 = 0101 0101
```

## CRC32 for the Firmware Image

Firmware API:

```c
uint32_t crc_util_crc32_init(void);
uint32_t crc_util_crc32_update(uint32_t crc, const uint8_t *data, uint32_t len);
uint32_t crc_util_crc32_finish(uint32_t crc);
```

Algorithm parameters:

| Property | Value |
| --- | --- |
| Reflected polynomial | `0xEDB88320` |
| Initial value | `0xFFFFFFFF` |
| Final XOR | `0xFFFFFFFF` |
| Processing model | Streaming chunk updates |
| Width | 32 bits |

The Python host uses:

```python
zlib.crc32(image) & 0xFFFFFFFF
```

The firmware uses a streaming CRC32 calculation. On START_OTA, the controller initializes the CRC32 state. On each DATA packet, the payload is added to the CRC32 state after it is written to flash. On END_OTA, the controller finalizes the CRC32 value and compares it against the expected CRC32 sent by the host.

Firmware flow:

```text
START_OTA:
  crc32_state = crc_util_crc32_init()

DATA:
  crc32_state = crc_util_crc32_update(crc32_state, payload, len)

END_OTA:
  calculated_crc32 = crc_util_crc32_finish(crc32_state)
  compare calculated_crc32 with expected_crc32 from START_OTA
```

If CRC32 does not match, the firmware aborts the OTA session and does not select the new boot partition. The previous firmware remains the active boot target.

## Why Both CRC16 and CRC32 Are Used

CRC16 is cheap and effective for short UART packets. It allows a failed packet to be rejected immediately and retried without restarting the full firmware transfer.

CRC32 checks the firmware image as a complete object. It catches full-image mismatch, incorrect files, write-path issues, and any transfer inconsistency that was not caught at packet level.

Using both checks gives the OTA system early packet-level recovery and final image-level confidence before boot partition switching.
