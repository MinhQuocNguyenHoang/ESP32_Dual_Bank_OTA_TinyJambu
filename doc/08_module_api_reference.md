# API reference theo module

File này tóm tắt các API nội bộ quan trọng của firmware. Mục tiêu là giúp đọc code nhanh hơn khi cần giải thích từng layer trong buổi demo hoặc review.

## `crc_until`

Header: `main/ota/crc_until.h`

`crc_util_crc16_ccitt(const uint8_t *data, uint32_t len)` tính CRC16/CCITT-FALSE cho một buffer liên tục. UART protocol dùng hàm này để kiểm tra toàn vẹn từng frame. Nếu `data == NULL`, implementation hiện tại không xử lý byte nào và trả về giá trị khởi tạo.

`crc_util_crc32_init(void)` trả về accumulator ban đầu của CRC32, hiện là `0xFFFFFFFF`.

`crc_util_crc32_update(uint32_t crc, const uint8_t *data, uint32_t len)` cập nhật CRC32 accumulator với một chunk dữ liệu. OTA controller gọi hàm này sau mỗi packet DATA đã ghi thành công vào flash.

`crc_util_crc32_finish(uint32_t crc)` áp dụng final XOR `0xFFFFFFFF` để tạo CRC32 cuối cùng. Giá trị này được so sánh với CRC32 mà Python host gửi trong START_OTA.

## `uart_proto`

Header: `main/ota/uart_proto.h`

`uart_proto_init(const uart_proto_config_t *config)` cấu hình UART driver, set pin TX/RX, install UART driver và tạo event queue. Layer này dùng ESP-IDF UART queue, phù hợp yêu cầu nhận UART theo hướng interrupt-driven thay vì polling thanh ghi.

`uart_proto_receive_packet(uart_proto_packet_t *packet, TickType_t timeout_ticks)` chờ một packet UART, tìm magic bytes, đọc header, đọc payload, kiểm CRC16 và trả packet đã decode. Hàm này chỉ trả `ESP_OK` khi packet hợp lệ ở tầng frame.

`uart_proto_send_ack(uint16_t seq, uart_proto_status_t status)` gửi ACK response frame cho sequence tương ứng.

`uart_proto_send_nack(uint16_t seq, uart_proto_status_t status)` gửi NACK response frame cho sequence tương ứng.

`uart_proto_deinit(void)` xóa UART driver và reset trạng thái init của protocol layer.

## `ota_controller`

Header: `main/ota/ota_controller.h`

`ota_controller_init(const ota_controller_config_t *config)` reset controller context, lưu cấu hình runtime và init UART protocol. Sau khi hàm này thành công, task OTA có thể gọi `ota_controller_process_once()` liên tục.

`ota_controller_process_once(void)` xử lý một packet OTA. Hàm này nhận packet qua `uart_proto`, dispatch sang handler START/DATA/END/ABORT, gửi ACK/NACK và reboot nếu END_OTA thành công.

`ota_controller_get_state(void)` trả về state hiện tại của OTA controller. State này có thể dùng để debug hoặc mở rộng hiển thị trạng thái OTA lên OLED.

`ota_controller_deinit(void)` abort session nếu cần, deinit UART protocol và clear controller context.

## `ota_writer`

Header: `main/ota/ota_writer.h`

`ota_writer_begin(ota_writer_context_t *ctx, uint32_t image_size)` tìm inactive OTA partition, kiểm tra image size, gọi `esp_ota_begin()` và mở một OTA write session.

`ota_writer_write(ota_writer_context_t *ctx, const uint8_t *data, uint32_t len)` ghi chunk firmware tiếp theo bằng `esp_ota_write()`. Hàm này kiểm soát tổng byte đã ghi để không vượt quá image size.

`ota_writer_finish(ota_writer_context_t *ctx, bool set_boot_partition)` kết thúc session bằng `esp_ota_end()`. Nếu `set_boot_partition == true`, hàm gọi `esp_ota_set_boot_partition()` để yêu cầu bootloader boot image mới sau reset.

`ota_writer_abort(ota_writer_context_t *ctx)` hủy session OTA đang active bằng `esp_ota_abort()`.

`ota_writer_mark_running_valid(void)` gọi `esp_ota_mark_app_valid_cancel_rollback()` để xác nhận firmware hiện tại chạy ổn và hủy trạng thái rollback pending.

`ota_writer_mark_running_invalid_and_rollback(void)` gọi `esp_ota_mark_app_invalid_rollback_and_reboot()` để đánh dấu firmware hiện tại lỗi và reboot về image cũ.

`ota_writer_restart(void)` gọi `esp_restart()` sau khi OTA thành công.

## `nvs_store`

Header: `main/ota/nvs_store.h`

`nvs_store_set_ota_pending(void)` đặt cờ pending trong NVS và reset boot counter. Hàm này được gọi trước khi firmware mới được set làm boot partition.

`nvs_store_clear_ota_pending(void)` xóa cờ pending và reset boot counter sau khi firmware mới được xác nhận hợp lệ.

`nvs_store_is_ota_pending(bool *pending)` đọc cờ pending từ NVS. Nếu key chưa tồn tại, trạng thái mặc định là false.

`nvs_store_get_ota_boot_count(uint32_t *count)` đọc số lần boot của image đang pending. Nếu key chưa tồn tại, giá trị mặc định là 0.

`nvs_store_increment_ota_boot_count(uint32_t *count)` tăng boot counter và trả lại giá trị mới nếu caller truyền pointer hợp lệ.

`nvs_store_reset_ota_boot_count(void)` đưa boot counter về 0.

## `glucose_monitor`

Header: `main/app/glucose_monitor.h`

`glucose_monitor_init(void)` khởi tạo network stack, event loop, buzzer, MAX30102, SSD1306, WiFi và MQTT. Hàm này cũng cập nhật OLED theo từng giai đoạn boot.

`glucose_monitor_process(void)` chạy state machine đo glucose. Hàm này được gọi lặp trong `app_main()` với delay 10 ms.

## `ota_test_app`

Header: `main/app/ota_test_app.h`

`ota_test_app_init(void)` in thông tin firmware test OTA, version được truyền từ CMake và partition đang chạy. Hàm này dùng khi build với `APP_OTA_TEST_MODE=ON`.

`ota_test_app_process(void)` in heartbeat định kỳ và free heap. Hàm này giúp xác nhận firmware mới vẫn đang chạy sau khi OTA reboot.

## `max30102`

Header: `main/drivers/max30102.h`

`max30102_init(i2c_port_t i2c_num, gpio_num_t sda_pin, gpio_num_t scl_pin)` khởi tạo I2C driver và cấu hình cảm biến MAX30102.

`max30102_check(void)` đọc số mẫu mới trong FIFO cảm biến và đưa vào circular buffer nội bộ.

`max30102_read_fifo(max30102_sample_t *sample)` cập nhật FIFO rồi trả sample Red/IR mới nhất.

`max30102_get_ir(uint32_t *ir_val)` cập nhật FIFO rồi trả giá trị IR mới nhất, dùng cho phát hiện ngón tay.

`max30102_reset_fifo(void)` reset các FIFO pointer của MAX30102.
