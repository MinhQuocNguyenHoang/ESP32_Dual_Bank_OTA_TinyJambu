# Tài liệu kỹ thuật dự án Non-Invasive Glucose Monitoring

Tài liệu này mô tả kiến trúc, luồng xử lý và cách vận hành của dự án đo glucose không xâm lấn trên ESP32. Phần OTA được viết theo yêu cầu trong đề bài: cập nhật firmware qua UART, dùng dual-bank flash, kiểm tra CRC16 theo từng gói, kiểm tra CRC32 cho toàn bộ firmware, dùng ESP-IDF OTA API chính thức và có cơ chế rollback dựa trên NVS.

Các file trong thư mục này được chia theo từng lớp kỹ thuật để dễ đọc và dễ bảo trì.

| File | Nội dung |
| --- | --- |
| [01_project_overview.md](01_project_overview.md) | Tổng quan hệ thống, chức năng chính, cấu trúc mã nguồn và vai trò từng module. |
| [02_firmware_architecture.md](02_firmware_architecture.md) | Kiến trúc firmware ESP32, task chính, state machine đo glucose và cách các layer giao tiếp. |
| [03_uart_ota_protocol.md](03_uart_ota_protocol.md) | Giao thức UART OTA, định dạng frame, command, ACK/NACK, sequence và wiring demo. |
| [04_crc_integrity.md](04_crc_integrity.md) | CRC16/CCITT-FALSE cho packet và CRC32 cho toàn bộ firmware image. |
| [05_ota_flash_and_rollback.md](05_ota_flash_and_rollback.md) | OTA writer, partition dual-bank, boot partition, NVS pending flag, boot counter và rollback. |
| [06_host_tools_and_demo.md](06_host_tools_and_demo.md) | Hướng dẫn build, flash lần đầu, gửi firmware bằng Python, cách xem log và xử lý lỗi thường gặp. |
| [07_glucose_pipeline.md](07_glucose_pipeline.md) | Luồng đo PPG từ MAX30102, trích xuất đặc trưng, suy luận mô hình, mã hóa TinyJAMBU và gửi MQTT. |
| [08_module_api_reference.md](08_module_api_reference.md) | Tóm tắt API nội bộ theo từng module firmware. |
| [09_requirement_traceability.md](09_requirement_traceability.md) | Đối chiếu yêu cầu đề bài với implementation hiện tại. |
| [10_ota_test_firmware.md](10_ota_test_firmware.md) | Firmware test tối giản và script wrapper để build/gửi OTA bằng Python. |

Tên project ESP-IDF hiện tại là `glucose_monitor_espidf`. Binary OTA sinh ra sau khi build nằm tại:

```text
build/glucose_monitor_espidf.bin
```

Firmware đang dùng UART2 cho OTA:

```text
ESP32 UART2 RX: GPIO26
ESP32 UART2 TX: GPIO27
Baud rate: 115200
```

Log/flash mặc định vẫn đi qua UART0 của board ESP32. Khi demo đầy đủ nên dùng hai đường serial riêng: một cổng để nạp và xem log, một USB-UART rời để truyền file OTA qua GPIO26/GPIO27.
