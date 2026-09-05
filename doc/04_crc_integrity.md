# CRC và kiểm tra toàn vẹn dữ liệu

Project dùng hai loại CRC ở hai tầng khác nhau. CRC16/CCITT-FALSE bảo vệ từng packet UART. CRC32 bảo vệ toàn bộ firmware image sau khi truyền xong. Hai phép kiểm tra này không thay thế nhau, vì chúng giải quyết hai loại lỗi khác nhau.

CRC16 phát hiện lỗi sớm trên từng frame nhỏ. Nếu một byte trong packet bị sai, ESP32 có thể NACK ngay packet đó để host gửi lại. CRC32 phát hiện lỗi ở mức toàn file. Nếu mọi packet đều qua CRC16 nhưng dữ liệu cuối cùng vẫn không đúng với file gốc, CRC32 ở cuối session sẽ chặn firmware lỗi trước khi set boot partition.

## CRC16/CCITT-FALSE

Hàm triển khai trong project:

```c
uint16_t crc_util_crc16_ccitt(const uint8_t *data, uint32_t len);
```

Thông số thuật toán:

| Thuộc tính | Giá trị |
| --- | --- |
| Tên thường dùng | CRC-16/CCITT-FALSE |
| Polynomial | `0x1021` |
| Initial value | `0xFFFF` |
| Input reflection | Không |
| Output reflection | Không |
| Final XOR | Không |
| Độ rộng | 16 bit |

Với UART packet, CRC16 được tính trên header và payload:

```text
[MAGIC0][MAGIC1][CMD][SEQ][LEN][PAYLOAD]
```

Hai byte CRC16 cuối frame không được đưa vào dữ liệu đầu vào của phép tính. Sau khi tính xong, CRC16 được encode little-endian trong frame.

Ví dụ frame START có payload 8 byte. Dữ liệu đưa vào CRC16 là:

```text
A5 5A 01 00 00 08 00 <image_size 4 byte> <crc32 4 byte>
```

Trong đó:

```text
A5 5A       magic
01          command START_OTA
00 00       sequence = 0
08 00       payload length = 8
```

Firmware nhận frame, tính lại CRC16 trên cùng vùng dữ liệu. Nếu giá trị nhận được khác giá trị tự tính, frame bị loại và controller trả NACK `CRC_ERROR`.

## Từng bước tính CRC16 trong code

Thuật toán xử lý từng byte. Với mỗi byte, byte đó được đưa vào 8 bit cao của thanh ghi CRC:

```c
crc = crc ^ (data[i] << 8);
```

Sau đó lặp 8 lần, tương ứng 8 bit:

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

Bit `0x8000` là bit cao nhất của thanh ghi 16 bit. Nếu bit cao nhất đang là 1, sau khi shift trái phải XOR với polynomial `0x1021`. Nếu bit cao nhất là 0, chỉ shift trái. Kết quả được ép về `uint16_t`, nên chỉ giữ lại 16 bit thấp.

Điểm quan trọng là XOR không phải phép cộng số học. XOR làm việc theo từng bit:

```text
0 xor 0 = 0
0 xor 1 = 1
1 xor 0 = 1
1 xor 1 = 0
```

Do đó:

```text
0xFFFF xor 0xAA00 = 0x55FF
```

Vì byte cao:

```text
0xFF xor 0xAA = 0x55
1111 1111 xor 1010 1010 = 0101 0101
```

## CRC32 cho toàn bộ firmware image

Hàm triển khai trong project:

```c
uint32_t crc_util_crc32_init(void);
uint32_t crc_util_crc32_update(uint32_t crc, const uint8_t *data, uint32_t len);
uint32_t crc_util_crc32_finish(uint32_t crc);
```

Thông số thuật toán:

| Thuộc tính | Giá trị |
| --- | --- |
| Polynomial reflected | `0xEDB88320` |
| Initial value | `0xFFFFFFFF` |
| Final XOR | `0xFFFFFFFF` |
| Kiểu xử lý | Streaming, update theo chunk |
| Độ rộng | 32 bit |

Host Python dùng:

```python
zlib.crc32(image) & 0xFFFFFFFF
```

Firmware dùng CRC32 streaming. Khi nhận `START_OTA`, controller gọi `crc_util_crc32_init()`. Sau mỗi packet `DATA`, controller gọi `crc_util_crc32_update()` với payload vừa ghi vào flash. Khi nhận `END_OTA`, controller gọi `crc_util_crc32_finish()` để lấy CRC32 cuối cùng.

Luồng trong firmware:

```text
START_OTA:
  crc32_state = crc_util_crc32_init()

DATA:
  crc32_state = crc_util_crc32_update(crc32_state, payload, len)

END_OTA:
  calculated_crc32 = crc_util_crc32_finish(crc32_state)
  compare calculated_crc32 with expected_crc32 from START_OTA
```

Nếu CRC32 không khớp, firmware gọi `ota_writer_abort()` và không set boot partition mới. Đây là chốt bảo vệ cuối cùng để tránh boot vào firmware bị lỗi trong quá trình truyền.

## Vì sao cần cả CRC16 và CRC32

CRC16 giúp phát hiện lỗi ngay tại packet đang truyền. Với packet tối đa 256 byte, việc tính CRC16 nhanh, frame ngắn và phản hồi ACK/NACK đơn giản. Nếu lỗi xảy ra ở packet thứ 10, host chỉ cần gửi lại packet thứ 10, không cần gửi lại toàn bộ file.

CRC32 kiểm tra toàn bộ firmware image như một đối tượng hoàn chỉnh. Nếu có lỗi logic trong sequence, lỗi ghi flash, lỗi host gửi nhầm file, hoặc lỗi nào đó không bị bắt ở từng packet, CRC32 cuối session vẫn có cơ hội phát hiện.

Trong hệ thống OTA, cách kết hợp này thực tế hơn chỉ dùng một CRC duy nhất. CRC16 giảm chi phí retry trong lúc truyền, CRC32 quyết định image có đủ tin cậy để boot hay không.
