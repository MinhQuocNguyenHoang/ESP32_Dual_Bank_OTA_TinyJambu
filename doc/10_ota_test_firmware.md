# Firmware test OTA và script demo Python

Project có thêm một build mode tối giản để kiểm tra OTA độc lập với pipeline đo glucose. Mục tiêu của mode này là tạo một firmware nhỏ, boot nhanh, không phụ thuộc WiFi, MQTT, sensor hoặc OLED. Sau khi OTA thành công, firmware test in version và partition đang chạy để xác nhận ESP32 đã boot vào image mới.

## Vì sao cần firmware test riêng

Firmware đầy đủ của project khởi tạo MAX30102, OLED, WiFi, MQTT, TinyJAMBU và state machine đo glucose. Khi demo OTA, các phần này có thể làm việc kiểm thử chậm hơn hoặc gây nhiễu khi debug, ví dụ WiFi chờ timeout hoặc sensor chưa nối.

Firmware test OTA chỉ giữ lại các phần cần cho OTA:

```text
NVS init
rollback health check
OTA controller task
version log
heartbeat log
```

Nhờ vậy có thể kiểm tra nhanh ba điều quan trọng:

```text
file mới được nhận qua UART
ESP32 đã ghi sang OTA slot còn lại
bootloader đã boot vào image mới sau reset
```

## Source code test

Source test nằm tại:

```text
main/app/ota_test_app.c
main/app/ota_test_app.h
```

Khi firmware test chạy, log có dạng:

```text
Minimal OTA test firmware started
Version: ota-demo-v2
Running partition: ota_1 at 0x00200000
OTA test heartbeat=1, free_heap=... bytes, version=ota-demo-v2
```

Version được truyền từ CMake bằng macro `APP_OTA_TEST_VERSION`. Nhờ vậy mỗi lần build test có thể đặt version khác nhau mà không cần sửa source code.

## Build firmware test thủ công

Có thể build firmware test bằng ESP-IDF:

```bash
idf.py -B build_ota_test \
  -DAPP_OTA_TEST_MODE=ON \
  -DAPP_OTA_TEST_VERSION=ota-demo-v2 \
  build
```

Binary sinh ra tại:

```text
build_ota_test/glucose_monitor_espidf.bin
```

Tên project vẫn là `glucose_monitor_espidf`, chỉ khác nội dung firmware vì CMake đang bật test mode.

## Gửi firmware test bằng script wrapper

Script wrapper nằm tại:

```text
tools/ota_demo_flash.py
```

Lệnh build firmware test rồi gửi qua UART OTA:

```bash
./tools/ota_test.sh /dev/ttyUSB1 ota-demo-v2
```

Script sẽ thực hiện hai bước:

```text
idf.py -B build_ota_test -DAPP_OTA_TEST_MODE=ON -DAPP_OTA_TEST_VERSION=... build
python3 tools/ota_uart_send.py --port ... --file build_ota_test/glucose_monitor_espidf.bin
```

Muốn đổi thư mục build:

```bash
./tools/build_ota_test.sh ota-demo-v3 build_ota_v3

python3 tools/ota_demo_flash.py \
  --port /dev/ttyUSB1 \
  --file build_ota_v3/glucose_monitor_espidf.bin
```

## Gửi firmware có sẵn

Nếu đã build firmware trước đó, có thể gửi file `.bin` bất kỳ:

```bash
./tools/ota_full.sh /dev/ttyUSB1 build/glucose_monitor_espidf.bin
```

Nếu không truyền `--file`, script mặc định dùng:

```text
build/glucose_monitor_espidf.bin
```

## Quy trình demo đề xuất

Trước tiên flash firmware đầy đủ có OTA bằng cổng nạp bình thường:

```bash
./tools/flash_initial.sh /dev/ttyUSB0
```

Đợi log:

```text
OTA task started on UART2 RX=26 TX=27
```

Sau đó mở terminal khác và gửi firmware test:

```bash
./tools/ota_test.sh /dev/ttyUSB1 ota-demo-v2
```

Sau khi OTA hoàn tất, ESP32 reboot. Trên monitor cần thấy version test mới. Nếu lần sau muốn quay lại firmware đầy đủ, build bình thường rồi gửi:

```bash
idf.py build

./tools/ota_full.sh /dev/ttyUSB1
```

## Lưu ý về môi trường

Các shell script `ota_test.sh`, `build_ota_test.sh` và `flash_initial.sh` sẽ tự source ESP-IDF nếu tìm thấy `$IDF_PATH/export.sh` hoặc `$HOME/Tools/esp/esp-idf/export.sh`. Nếu gọi trực tiếp `ota_demo_flash.py` và script cần build firmware, terminal phải source ESP-IDF trước:

```bash
source /home/minhquocnguyenhoang/Tools/esp/esp-idf/export.sh
```

Nếu chỉ dùng `--file` để gửi binary có sẵn, script không cần gọi `idf.py`, nhưng vẫn cần `pyserial` vì quá trình gửi OTA dùng UART.

Firmware test vẫn giữ OTA task, NVS pending flag và rollback health check. Vì vậy nó vẫn kiểm tra được OTA theo đúng cơ chế chính của project, chỉ bỏ phần ứng dụng đo glucose để việc demo gọn hơn.
