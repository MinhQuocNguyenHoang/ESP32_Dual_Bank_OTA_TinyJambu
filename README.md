# Non-Invasive Glucose Monitoring

Đây là project ESP-IDF cho hệ thống đo glucose không xâm lấn dùng ESP32, MAX30102, OLED SSD1306, TinyJAMBU, MQTT và UART OTA dual-bank.

Bộ tài liệu kỹ thuật chính nằm trong thư mục [doc](doc/README.md). Nên bắt đầu từ [doc/README.md](doc/README.md), sau đó đọc tiếp theo thứ tự số file.

Lệnh build firmware chính:

```bash
idf.py build
```

Binary dùng để gửi OTA:

```text
build/glucose_monitor_espidf.bin
```

OTA UART hiện dùng UART2 với GPIO26 là RX và GPIO27 là TX. Lần đầu tiên cần flash firmware có OTA bằng cổng nạp thông thường, sau đó mới gửi các bản cập nhật tiếp theo bằng `tools/ota_uart_send.py`.

Flash lần đầu firmware có OTA:

```bash
./tools/flash_initial.sh /dev/ttyUSB0
```

Gửi firmware chính đã build sẵn qua OTA:

```bash
./tools/ota_full.sh /dev/ttyUSB1
```

Build firmware test tối giản rồi gửi qua OTA:

```bash
./tools/ota_test.sh /dev/ttyUSB1 ota-demo-v2
```

Firmware test chỉ in version, running partition và heartbeat qua log. Nó dùng để kiểm tra nhanh OTA đã boot sang image mới hay chưa.
