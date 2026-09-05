# Tổng quan dự án

Dự án xây dựng một hệ thống đo glucose không xâm lấn dùng ESP32, cảm biến quang học MAX30102, màn hình OLED SSD1306, mô hình học máy nhúng và kênh truyền MQTT. Ngoài chức năng đo và gửi dữ liệu, firmware còn có cơ chế cập nhật OTA qua UART theo giao thức tự thiết kế.

Mục tiêu kỹ thuật của phần nhúng là gom nhiều khối chức năng độc lập vào một firmware ESP-IDF thuần C. Các khối chính gồm thu thập tín hiệu PPG, xử lý đặc trưng, suy luận mô hình, mã hóa telemetry bằng TinyJAMBU, gửi MQTT, hiển thị OLED, điều khiển buzzer và cập nhật firmware qua UART dual-bank.

## Chức năng chính

Hệ thống đo tín hiệu Red và IR từ MAX30102 qua I2C. Khi phát hiện ngón tay, firmware ổn định tín hiệu trong một khoảng thời gian cố định, lấy mẫu PPG, tính các đặc trưng đơn giản từ buffer, đưa đặc trưng vào mô hình Gradient Boosting đã export sang header C, sau đó hiển thị kết quả glucose lên OLED.

Kết quả đo được mã hóa bằng TinyJAMBU-128 AEAD rồi publish qua MQTT. Phía Python web dashboard nhận telemetry, giải mã payload, lưu lịch sử vào SQLite và gửi ACK ngược lại để firmware biết dữ liệu đã đến gateway.

Phần OTA cho phép PC gửi file firmware `.bin` qua UART. ESP32 nhận file theo từng packet nhỏ, kiểm tra CRC16 từng packet, ghi vào phân vùng OTA không hoạt động, kiểm tra CRC32 toàn bộ image, đặt phân vùng mới làm boot partition rồi reset.

## Cấu trúc mã nguồn

| Đường dẫn | Vai trò |
| --- | --- |
| `main/main.c` | Entry point của firmware, khởi tạo NVS, kiểm tra rollback, khởi tạo glucose monitor và tạo task OTA. |
| `main/app/glucose_monitor.c` | State machine đo glucose, OLED UI, WiFi, MQTT, mã hóa TinyJAMBU và upload telemetry. |
| `main/app/ota_test_app.c` | App tối giản dùng để build firmware test OTA và xác nhận version sau reboot. |
| `main/drivers/max30102.c` | Driver I2C cho MAX30102, đọc FIFO Red/IR và lưu mẫu vào circular buffer. |
| `main/drivers/ssd1306.c` | Driver OLED SSD1306 dùng để hiển thị trạng thái đo và kết quả. |
| `main/model/gradient_boosting_model.h` | Mô hình Gradient Boosting đã được export sang code C header. |
| `main/ota/crc_until.c` | CRC utility: CRC16/CCITT-FALSE cho packet và CRC32 reflected cho firmware image. |
| `main/ota/uart_proto.c` | Layer giao thức UART packet: init UART, nhận frame, kiểm CRC16, gửi ACK/NACK. |
| `main/ota/ota_controller.c` | State machine OTA: START/DATA/END/ABORT, kiểm sequence, cập nhật CRC32, gọi OTA writer. |
| `main/ota/ota_writer.c` | Wrapper quanh ESP-IDF OTA API: begin, write, end, set boot partition, abort, rollback. |
| `main/ota/nvs_store.c` | Lưu trạng thái pending OTA và số lần boot thử firmware mới trong NVS. |
| `tools/ota_uart_send.py` | Script Python phía PC để gửi firmware `.bin` qua UART theo protocol của project. |
| `tools/ota_demo_flash.py` | Python wrapper để build firmware thường hoặc firmware test rồi gọi `ota_uart_send.py`. |
| `tools/*.sh` | Shell scripts rút gọn lệnh flash lần đầu, build OTA test, gửi OTA test và gửi firmware chính. |
| `app/app.py` | Web dashboard Python, nhận MQTT telemetry, giải mã TinyJAMBU, lưu SQLite và emit WebSocket. |
| `app/Gateway.py` | MQTT bridge từ local broker lên ThingsBoard. |
| `software/train_model.py` | Script huấn luyện mô hình từ dữ liệu mẫu. |
| `partitions.csv` | Partition table custom có `ota_0`, `ota_1`, `otadata`, `nvs` và `phy_init`. |

## Ranh giới giữa các layer

Phần OTA được tách layer rõ ràng để tránh một module làm quá nhiều việc. `uart_proto` chỉ biết UART frame và CRC16 packet. `ota_controller` chỉ biết state machine OTA và quyết định packet nào hợp lệ. `ota_writer` chỉ làm việc với flash thông qua ESP-IDF OTA API. `nvs_store` chỉ lưu bookkeeping cho rollback. `crc_until` không biết UART hay OTA, nó chỉ cung cấp thuật toán CRC.

Cách tách này giúp phần nhận UART không phụ thuộc trực tiếp vào cách ghi flash, và phần ghi flash không cần biết packet đến từ đâu. Nếu sau này thay UART bằng BLE, SPI hoặc TCP, logic OTA controller và OTA writer vẫn có thể giữ gần như nguyên vẹn.

## Phần cứng đang dùng

Firmware hiện tại giả định các kết nối chính như sau.

| Chức năng | Giao tiếp | Chân ESP32 |
| --- | --- | --- |
| MAX30102 | I2C0 | SDA GPIO32, SCL GPIO33 |
| SSD1306 OLED | I2C0 | SDA GPIO32, SCL GPIO33 |
| Buzzer | LEDC PWM | GPIO25 |
| OTA UART | UART2 | RX GPIO26, TX GPIO27 |
| Log/flash mặc định | UART0 | Theo USB-UART onboard của board ESP32 |

Nếu dùng USB-UART rời để OTA, dây phải được nối chéo tín hiệu:

```text
USB-UART TX -> ESP32 GPIO26
USB-UART RX -> ESP32 GPIO27
USB-UART GND -> ESP32 GND
```

Mức logic UART phải là 3.3 V. Board ESP32 không nên nhận tín hiệu UART 5 V.

## Trạng thái hiện tại của project

Project hiện đã có phần OTA chạy end-to-end: firmware mới được gửi từ script Python qua UART, ESP32 ghi sang slot OTA còn lại, kiểm CRC32 và reboot vào firmware mới. Lần đầu tiên vẫn phải flash firmware có OTA qua cổng nạp thông thường. Sau đó các lần cập nhật tiếp theo có thể đi qua UART OTA.

Một điểm cần lưu ý khi demo là OTA task được tạo sau khi `glucose_monitor_init()` chạy xong. Hàm này có chờ WiFi và MQTT một khoảng thời gian, nên sau khi reset cần đợi log `OTA task started on UART2 RX=26 TX=27` trước khi chạy script gửi OTA.

Project cũng có một build mode test OTA tối giản. Khi bật `APP_OTA_TEST_MODE`, firmware không chạy pipeline đo glucose mà chỉ in version, partition hiện tại và heartbeat. Mode này dùng để demo cập nhật version qua OTA nhanh hơn, không phụ thuộc WiFi, MQTT, sensor hoặc OLED.
