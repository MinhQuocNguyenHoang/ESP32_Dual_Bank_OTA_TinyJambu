# Công cụ host và quy trình demo OTA

Phía PC dùng script `tools/ota_uart_send.py` để gửi firmware binary qua UART. Script đọc file `.bin`, tính CRC32 toàn bộ file, tạo packet `START_OTA`, chia firmware thành nhiều packet `DATA`, gửi packet `END_OTA`, và chờ ACK/NACK sau mỗi packet.

Script này dùng `pyserial`, vì vậy môi trường Python cần có package `serial`.

## Build firmware

Từ thư mục gốc project:

```bash
idf.py build
```

Binary dùng cho OTA nằm ở:

```text
build/glucose_monitor_espidf.bin
```

Nếu mới bật OTA lần đầu hoặc board đang chạy firmware cũ chưa có OTA task, cần flash bằng cổng nạp bình thường:

```bash
./tools/flash_initial.sh /dev/ttyUSB0
```

Sau khi firmware mới chạy, chờ log:

```text
OTA task started on UART2 RX=26 TX=27
```

Chỉ sau thời điểm này host mới nên gửi OTA.

## Demo với hai UART

Đây là cách demo rõ ràng nhất.

```text
UART0 onboard:
  dùng để flash lần đầu và xem log bằng idf.py monitor

UART2 GPIO26/GPIO27:
  dùng để nhận firmware OTA từ script Python
```

Terminal xem log:

```bash
idf.py -p /dev/ttyUSB0 monitor
```

Terminal gửi OTA:

```bash
./tools/ota_full.sh /dev/ttyUSB1
```

Port `/dev/ttyUSB1` chỉ là ví dụ. Cần thay bằng port thật của USB-UART rời nối vào GPIO26/GPIO27.

## Cách xác định đúng serial port

Trên Linux, có thể kiểm tra danh sách serial:

```bash
ls /dev/ttyUSB* /dev/ttyACM*
```

Một cách chắc hơn là rút USB-UART rời ra, chạy lệnh trên, sau đó cắm lại và chạy lại. Port mới xuất hiện là port của USB-UART rời.

Nếu `idf.py monitor` xem log được trên `/dev/ttyUSB0`, thông thường `/dev/ttyUSB0` là UART0 onboard của board ESP32. OTA hiện tại không nghe UART0, nên script Python phải chạy trên port của USB-UART nối GPIO26/GPIO27.

## Tham số script Python

Script hỗ trợ các tham số chính:

| Tham số | Ý nghĩa |
| --- | --- |
| `--port` | Serial port dùng để gửi OTA. |
| `--file` | Đường dẫn file firmware `.bin`. |
| `--baud` | Baud rate, mặc định 115200. |
| `--chunk-size` | Số byte firmware trong mỗi DATA packet, tối đa 256. |
| `--timeout` | Thời gian chờ ACK/NACK cho mỗi packet. |
| `--retries` | Số lần retry mỗi packet khi timeout hoặc NACK. |
| `--settle-delay` | Thời gian chờ sau khi mở serial port trước khi gửi. |

Lệnh thường dùng:

```bash
./tools/ota_full.sh /dev/ttyUSB1
```

Project còn có các script wrapper trong thư mục `tools/`. Các script này gom những lệnh dài thành lệnh ngắn hơn để demo dễ lặp lại.

| Script | Công dụng |
| --- | --- |
| `tools/flash_initial.sh` | Flash firmware bằng ESP-IDF qua cổng nạp và mở monitor. |
| `tools/build_ota_test.sh` | Chỉ build firmware test OTA tối giản. |
| `tools/ota_test.sh` | Build firmware test OTA rồi gửi qua UART OTA. |
| `tools/ota_full.sh` | Gửi firmware chính đã build sẵn qua UART OTA. |

Gửi file `.bin` mặc định trong thư mục build:

```bash
./tools/ota_full.sh /dev/ttyUSB1
```

Gửi một file `.bin` chỉ định:

```bash
./tools/ota_full.sh /dev/ttyUSB1 build/glucose_monitor_espidf.bin
```

Build firmware test OTA tối giản rồi gửi:

```bash
./tools/ota_test.sh /dev/ttyUSB1 ota-demo-v2
```

Khi dùng `ota_test.sh`, script build vào thư mục `build_ota_test` với CMake option `APP_OTA_TEST_MODE=ON`. Firmware test không chạy glucose monitor, chỉ in version và heartbeat để kiểm tra image mới đã boot hay chưa.

Khi chạy, script in thông tin firmware:

```text
Firmware : build/glucose_monitor_espidf.bin
Size     : 979936 bytes
CRC32    : 0x6650F41F
Port     : /dev/ttyUSB1 @ 115200
```

Sau đó script hiển thị progress bar theo số byte đã gửi.

## Log kỳ vọng trên ESP32

Khi START thành công:

```text
START OTA: image_size=... expected_crc32=...
Running partition: ...
Writing OTA image to: ...
```

Trong lúc ghi:

```text
WRITING: .../... bytes (...%)
```

Khi kết thúc:

```text
VERIFY: calculating firmware CRC32
SUCCESS: OTA image verified and boot partition updated
REBOOT: restarting after successful OTA
```

Sau reboot vào image mới, nếu rollback đang bật, firmware cần confirm image:

```text
OTA image pending validation, boot attempt 1/3
OTA image confirmed valid
```

## Các lỗi thường gặp

Timeout ở `seq=0` nghĩa là host gửi START nhưng không nhận ACK/NACK. Lỗi này xảy ra trước khi ghi flash. Cần kiểm tra firmware đã flash bản có OTA chưa, OTA task đã started chưa, port có đúng USB-UART rời không, dây TX/RX có nối chéo không, GND có chung không và USB-UART có đang dùng mức 3.3 V không.

Timeout sau một vài packet DATA thường liên quan đến đường truyền UART không ổn định, dây dài, baud rate quá cao so với chất lượng module, hoặc ESP32 bị reset giữa quá trình OTA. Có thể thử giảm chunk size, tăng timeout hoặc kiểm tra nguồn cấp.

NACK `CRC_ERROR` ở packet thường do frame bị sai dữ liệu trên đường truyền hoặc host/firmware không dùng cùng thuật toán CRC16. Với code hiện tại, cả hai dùng CRC16/CCITT-FALSE polynomial `0x1021`.

NACK `LENGTH_ERROR` thường do payload length sai, chunk size vượt 256 byte, START payload không đúng 8 byte hoặc END có payload khác 0.

NACK `SEQ_ERROR` nghĩa là ESP32 đang chờ sequence khác. Có thể do host gửi lại từ đầu khi ESP32 vẫn còn session active, hoặc ESP32 reset giữa quá trình truyền.

Lỗi `Image too large` trên log ESP32 nghĩa là file `.bin` lớn hơn partition OTA inactive. Cần tăng size partition hoặc giảm kích thước firmware.

## Demo đề xuất khi báo cáo

Quy trình demo nên đi theo thứ tự rõ ràng:

```text
1. Flash firmware có OTA qua cổng nạp bình thường.
2. Mở monitor và chỉ ra log OTA task started.
3. Chạy script Python gửi file bin qua UART2.
4. Theo dõi progress trên Python và log START/WRITING/VERIFY/SUCCESS trên ESP32.
5. Quan sát ESP32 reboot.
6. Sau reboot, chỉ ra log image pending validation và image confirmed valid.
```

Nếu muốn chứng minh rollback, có thể tạo một firmware cố tình fail self-test hoặc không confirm valid. Khi boot counter vượt quá 3, firmware sẽ gọi rollback. Khi demo phần này cần cẩn thận để không tự khóa board vào vòng reset khó quan sát.

Nếu chỉ cần chứng minh OTA hoạt động, nên dùng firmware test tối giản trước. Sau khi đã chắc UART OTA ổn, mới gửi firmware đầy đủ của glucose monitor. Cách này giảm ảnh hưởng từ WiFi, MQTT, sensor và OLED trong lúc demo OTA.
