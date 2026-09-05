# Kiến trúc firmware ESP32

Firmware được viết theo ESP-IDF và tổ chức thành các module C riêng. `app_main()` trong `main/main.c` đóng vai trò điều phối khởi động. Sau khi boot, hệ thống khởi tạo NVS, kiểm tra trạng thái rollback, khởi tạo chức năng đo glucose, xác nhận image OTA nếu image mới chạy ổn, rồi tạo task OTA chạy song song với vòng lặp đo glucose.

## Luồng khởi động

Luồng khởi động hiện tại có thứ tự như sau.

```text
app_main
  -> app_nvs_init
  -> ota_boot_health_check
  -> glucose_monitor_init
  -> ota_confirm_running_app_if_pending
  -> xTaskCreate(ota_task)
  -> while true: glucose_monitor_process
```

`app_nvs_init()` gọi `nvs_flash_init()`. Nếu NVS báo hết trang hoặc khác version, firmware erase NVS rồi init lại. Đây là cách xử lý quen thuộc trong ESP-IDF để tránh lỗi khi layout NVS thay đổi trong quá trình phát triển.

`ota_boot_health_check()` kiểm tra xem firmware đang chạy có đang ở trạng thái cần xác nhận hay không. Trạng thái này được đọc từ hai nguồn: NVS của project và OTA state của ESP-IDF. Nếu có pending image, boot counter trong NVS được tăng lên. Nếu số lần boot vượt quá giới hạn, firmware gọi rollback API để quay về image cũ.

`glucose_monitor_init()` khởi tạo phần đo glucose, bao gồm network stack, event loop, buzzer, MAX30102, SSD1306, WiFi và MQTT. Hàm này có thể chờ WiFi tối đa khoảng 10 giây và MQTT khoảng 6 giây.

`ota_confirm_running_app_if_pending()` được gọi sau khi khởi tạo glucose monitor. Ý nghĩa của đoạn này là chỉ xác nhận image OTA mới nếu firmware đã boot được và đi qua được phần khởi tạo chính. Khi xác nhận thành công, firmware gọi `esp_ota_mark_app_valid_cancel_rollback()` và xóa pending flag trong NVS.

Cuối cùng, `ota_task` được tạo. Task này init UART2 và chạy vòng lặp xử lý từng packet OTA.

## Task chính và task OTA

Project có một luồng xử lý chính nằm trong `app_main()` và một task phụ cho OTA.

```text
Main app loop
  -> glucose_monitor_process()
  -> vTaskDelay(10 ms)

OTA task
  -> ota_controller_init()
  -> loop:
       ota_controller_process_once()
```

`glucose_monitor_process()` chạy state machine đo glucose. Đây là phần tương tác với sensor, OLED, buzzer, TinyJAMBU và MQTT.

`ota_controller_process_once()` chờ một packet UART trong thời gian timeout cấu hình. Nếu timeout, task xem đó là trạng thái idle bình thường và tiếp tục vòng lặp. Nếu nhận được packet hợp lệ, controller xử lý command rồi gửi ACK hoặc NACK về host.

Thiết kế này giúp OTA không block vòng đo glucose trong thời gian chờ UART. Ngược lại, quá trình gửi OTA vẫn có thể bị ảnh hưởng nếu hệ thống đang thực hiện các đoạn delay dài trong các task khác, nên khi demo nên để thiết bị ở trạng thái ổn định và chờ OTA task đã started.

## State machine đo glucose

Module `glucose_monitor.c` dùng enum `display_state_t` để quản lý quá trình đo.

| State | Vai trò |
| --- | --- |
| `STATE_IDLE` | Chờ người dùng đặt ngón tay. OLED hiển thị trạng thái sẵn sàng. |
| `STATE_STABILIZING` | Đợi tín hiệu IR ổn định trong 5 giây. Nếu rút tay thì quay về idle. |
| `STATE_SAMPLING` | Lấy mẫu Red/IR vào buffer với sample rate 100 Hz trong cửa sổ 2 giây. |
| `STATE_PREDICT` | Tính feature, chạy mô hình Gradient Boosting, mã hóa kết quả và hiển thị glucose. |
| `STATE_UPLOADING` | Gửi telemetry đã mã hóa qua MQTT và chờ ACK từ web gateway. |
| `STATE_WAIT_RELEASE` | Chờ người dùng rút tay trước khi cho phép đo lần tiếp theo. |

Các tham số chính:

| Tham số | Giá trị hiện tại | Ý nghĩa |
| --- | --- | --- |
| `FINGER_THRESHOLD` | 30000 | Ngưỡng IR để xem như có ngón tay. |
| `SAMPLE_WINDOW` | 2000 ms | Thời gian lấy mẫu PPG cho một lần đo. |
| `SAMPLE_RATE` | 100 Hz | Tần số lấy mẫu logic trong firmware. |
| `BUFFER_SIZE` | 200 mẫu | Số mẫu Red/IR dùng cho feature extraction. |
| `STABILIZE_WINDOW_MS` | 5000 ms | Thời gian chờ tín hiệu ổn định trước khi lấy mẫu. |

## Kiến trúc OTA trong firmware

OTA được chia thành bốn module nhỏ.

| Module | Trách nhiệm |
| --- | --- |
| `crc_until` | Cung cấp CRC16 và CRC32. Không phụ thuộc ESP-IDF. |
| `uart_proto` | Cấu hình UART, nhận frame, kiểm frame, gửi ACK/NACK. Không biết flash. |
| `ota_controller` | Quản lý state machine START/DATA/END/ABORT, sequence, CRC32 image. |
| `ota_writer` | Gọi ESP-IDF OTA API để ghi firmware vào partition không hoạt động. |

Quan hệ gọi hàm trong một packet OTA hợp lệ:

```text
Python host
  -> UART frame
  -> uart_proto_receive_packet
  -> crc_util_crc16_ccitt
  -> ota_controller_handle_packet
  -> ota_writer_begin/write/finish
  -> uart_proto_send_ack hoặc uart_proto_send_nack
```

`ota_controller` không ghi flash trực tiếp. Nó chỉ gọi `ota_writer`. `ota_writer` cũng không biết START/DATA/END là gì; nó chỉ quản lý một session ghi OTA tuần tự. Cách chia này đáp ứng ràng buộc không dùng global chia sẻ trực tiếp giữa các layer. Mỗi module có static context riêng và expose API rõ ràng qua header.

## Ghi chú về rule return

Trong các module OTA mới, code đi theo phong cách một biến `ret` và return ở cuối hàm. Cách này giúp đọc luồng lỗi rõ hơn khi project muốn tránh nhiều `return` giữa hàm. Một số file cũ như `max30102.c` và `glucose_monitor.c` vẫn có đoạn return sớm từ trước; khi refactor tiếp có thể đồng bộ style dần, nhưng phần OTA/NVS/CRC đã đi theo hướng nhất quán hơn.
