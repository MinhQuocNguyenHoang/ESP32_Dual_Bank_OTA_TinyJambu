# Giao thức UART OTA

Giao thức UART OTA của project là giao thức packet-based chạy giữa PC host và ESP32. PC đọc file firmware `.bin`, chia thành các packet nhỏ tối đa 256 byte payload, gửi lần lượt qua UART. ESP32 kiểm tra từng packet bằng CRC16, xử lý command, rồi trả ACK hoặc NACK.

Giao thức này không phải giao thức bootloader UART mặc định của Espressif. Nó chạy trong application firmware. Vì vậy ESP32 phải đang chạy firmware có OTA task, không phải đang ở chế độ BOOT download.

## Cấu hình UART

Firmware hiện tại cấu hình OTA UART trong `main/main.c`:

```text
UART port: UART_NUM_2
Baud rate: 115200
RX buffer: 4096 bytes
TX buffer: 512 bytes
Event queue size: 20
ESP32 RX: GPIO26
ESP32 TX: GPIO27
```

Kết nối với USB-UART rời:

```text
USB-UART TX -> ESP32 GPIO26
USB-UART RX -> ESP32 GPIO27
USB-UART GND -> ESP32 GND
```

UART dùng mức logic 3.3 V. Nếu USB-UART có jumper 5 V/3.3 V thì chọn 3.3 V. Nếu board ESP32 đã được cấp nguồn bằng USB chính, thường không cần nối VCC từ USB-UART rời.

## Frame host gửi sang ESP32

Mỗi frame host-to-device có định dạng:

```text
[MAGIC0:1][MAGIC1:1][CMD:1][SEQ:2][LEN:2][PAYLOAD:0..256][CRC16:2]
```

Các trường multi-byte được encode little-endian.

| Trường | Kích thước | Giá trị hoặc ý nghĩa |
| --- | ---: | --- |
| `MAGIC0` | 1 byte | `0xA5` |
| `MAGIC1` | 1 byte | `0x5A` |
| `CMD` | 1 byte | Command OTA |
| `SEQ` | 2 byte | Sequence number của packet |
| `LEN` | 2 byte | Độ dài payload |
| `PAYLOAD` | 0 đến 256 byte | Dữ liệu tùy command |
| `CRC16` | 2 byte | CRC16 của toàn bộ frame trừ chính trường CRC16 |

CRC16 được tính trên:

```text
MAGIC0 || MAGIC1 || CMD || SEQ || LEN || PAYLOAD
```

Không tính hai byte CRC cuối vào chính phép tính CRC.

## Command

| Command | Giá trị | Payload | Ý nghĩa |
| --- | ---: | --- | --- |
| `START_OTA` | `0x01` | 8 byte | Bắt đầu OTA session. Payload gồm image size và CRC32 kỳ vọng. |
| `DATA` | `0x02` | 1 đến 256 byte | Một đoạn firmware image. |
| `END_OTA` | `0x03` | 0 byte | Kết thúc truyền file, yêu cầu ESP32 verify CRC32 và set boot partition. |
| `ABORT` | `0x04` | 0 byte | Hủy OTA session đang chạy. |

Payload của `START_OTA`:

```text
[IMAGE_SIZE:4][EXPECTED_CRC32:4]
```

Cả hai trường đều little-endian. `IMAGE_SIZE` là tổng số byte của file `.bin`. `EXPECTED_CRC32` là CRC32 của toàn bộ file firmware do Python host tính trước khi gửi.

## Sequence number

Packet đầu tiên dùng `SEQ = 0`. Sau mỗi packet hợp lệ, sequence tăng thêm 1.

Luồng bình thường:

```text
START_OTA  seq=0
DATA       seq=1
DATA       seq=2
DATA       seq=3
...
END_OTA    seq=N
```

ESP32 lưu `expected_seq` trong `ota_controller`. Nếu nhận `DATA` hoặc `END_OTA` sai sequence, controller trả NACK với status `SEQ_ERROR`. Điều này tránh trường hợp host gửi thiếu packet, gửi lặp packet ngoài ý muốn hoặc dữ liệu đến sai thứ tự.

## ACK/NACK response

ESP32 trả response frame sau khi xử lý một packet.

```text
[MAGIC0:1][MAGIC1:1][RESP:1][SEQ:2][LEN:2][STATUS:1][CRC16:2]
```

| Trường | Giá trị |
| --- | --- |
| `RESP_ACK` | `0x79` |
| `RESP_NACK` | `0x1F` |
| `LEN` | `0x0001` |
| `STATUS` | Mã trạng thái xử lý |

Status code:

| Status | Giá trị | Ý nghĩa |
| --- | ---: | --- |
| `OK` | `0x00` | Packet đã được xử lý thành công. |
| `CRC_ERROR` | `0x01` | CRC16 packet sai hoặc CRC32 image sai. |
| `SEQ_ERROR` | `0x02` | Sequence không đúng với sequence firmware đang chờ. |
| `LENGTH_ERROR` | `0x03` | Payload length sai, vượt giới hạn hoặc không khớp image size. |
| `STATE_ERROR` | `0x04` | Command không hợp lệ với trạng thái OTA hiện tại. |
| `INTERNAL_ERROR` | `0x05` | Lỗi nội bộ như ghi flash thất bại. |

Python script chỉ tiếp tục gửi packet tiếp theo khi nhận được ACK status OK cho đúng sequence. Nếu timeout hoặc nhận NACK, script retry packet hiện tại. Số lần retry mặc định là 3.

## Luồng OTA hoàn chỉnh

```text
PC host                         ESP32
   |                              |
   | START_OTA(seq=0)             |
   |----------------------------->|
   |                              | kiểm CRC16 packet
   |                              | đọc image_size và expected_crc32
   |                              | esp_ota_begin partition inactive
   | ACK(seq=0)                   |
   |<-----------------------------|
   |                              |
   | DATA(seq=1, chunk 0)         |
   |----------------------------->|
   |                              | kiểm CRC16
   |                              | kiểm sequence
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
   |                              | kiểm tổng bytes
   |                              | crc32_finish
   |                              | so sánh expected_crc32
   |                              | esp_ota_end
   |                              | esp_ota_set_boot_partition
   | ACK(seq=N)                   |
   |<-----------------------------|
   |                              | reboot
```

## Các lỗi demo thường gặp

Nếu Python timeout ngay `seq=0`, ESP32 chưa ACK cho packet START. Nguyên nhân thường là firmware đang chạy chưa có OTA task, gửi nhầm port, dây TX/RX sai, thiếu GND chung, USB-UART đang ở mức 5 V, hoặc OTA task chưa started vì firmware còn chờ WiFi/MQTT.

Nếu ESP32 có log `START OTA` nhưng Python vẫn timeout, kiểm tra dây ESP32 TX GPIO27 về USB-UART RX. Trường hợp này host gửi được sang ESP32, nhưng đường phản hồi từ ESP32 về host có vấn đề.

Nếu Python nhận NACK `LENGTH_ERROR`, kiểm tra chunk size, START payload và image size. Chunk size không được vượt 256 byte.

Nếu Python nhận NACK `SEQ_ERROR`, host và firmware đang lệch sequence. Nguyên nhân có thể do retry logic, reset ESP32 giữa lúc truyền, hoặc gửi lại cùng một session khi firmware vẫn còn active.

Nếu `END_OTA` bị NACK `CRC_ERROR`, packet có thể truyền đủ nhưng CRC32 toàn file không khớp. Cần kiểm tra file `.bin`, đường truyền và thuật toán CRC32 giữa Python và firmware.
