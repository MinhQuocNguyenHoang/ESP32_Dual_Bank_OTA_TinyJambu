# Đối chiếu yêu cầu đề bài với implementation

File này dùng để kiểm tra xem các yêu cầu trong `doc/ota_doc.txt` đang được đáp ứng ở đâu trong project. Đây không phải checklist trang trí; nó là bảng traceability để khi review có thể chỉ thẳng từ yêu cầu sang module và hành vi tương ứng.

## Yêu cầu chức năng OTA

| Yêu cầu | Implementation hiện tại |
| --- | --- |
| Nhận firmware nhị phân từ PC qua UART | `tools/ota_uart_send.py` gửi `.bin`; ESP32 nhận qua `uart_proto_receive_packet()` trên UART2. |
| Giao thức UART tùy chỉnh dạng packet | Định dạng frame nằm trong `main/ota/uart_proto.h` và parser nằm trong `main/ota/uart_proto.c`. |
| Mỗi packet có ACK/NACK | `uart_proto_send_ack()` và `uart_proto_send_nack()` gửi response sau khi `ota_controller_process_once()` xử lý packet. |
| Payload mỗi packet tối đa 256 byte | `UART_PROTO_MAX_PAYLOAD_SIZE = 256U`; Python cũng giới hạn `MAX_PAYLOAD_SIZE = 256`. |
| START_OTA chứa kích thước image và CRC32 | `OTA_CONTROLLER_START_PAYLOAD_SIZE = 8U`; `ota_controller_handle_start()` đọc image size và expected CRC32. |
| DATA ghi firmware theo từng chunk | `ota_controller_handle_data()` gọi `ota_writer_write()` cho từng payload DATA. |
| END_OTA xác minh toàn bộ image | `ota_controller_handle_end()` kiểm tổng byte và so sánh CRC32. |
| ABORT hủy session | `ota_controller_handle_abort()` gọi `ota_writer_abort()` nếu session active. |

## Yêu cầu về CRC

| Yêu cầu | Implementation hiện tại |
| --- | --- |
| CRC16 polynomial `0x1021` cho từng packet | `crc_util_crc16_ccitt()` dùng polynomial `0x1021U`. |
| CRC16 kiểm header, sequence, length và payload | `uart_proto_decode_frame()` tính CRC16 trên frame trừ hai byte CRC cuối. |
| CRC32 cho toàn bộ firmware image | Python dùng `zlib.crc32`; firmware dùng `crc_util_crc32_init/update/finish`. |
| CRC32 được so sánh trước khi set boot partition | `ota_controller_handle_end()` chỉ gọi `ota_writer_finish(..., true)` sau khi CRC32 khớp. |

## Yêu cầu về ESP-IDF OTA API

| Yêu cầu | Implementation hiện tại |
| --- | --- |
| Không ghi flash trực tiếp | Không có ghi flash thô trong OTA flow; ghi thông qua `ota_writer.c`. |
| Dùng `esp_ota_begin()` | `ota_writer_begin()` gọi `esp_ota_begin(update_partition, image_size, &ctx->handle)`. |
| Dùng `esp_ota_write()` | `ota_writer_write()` gọi `esp_ota_write(ctx->handle, data, len)`. |
| Dùng `esp_ota_end()` | `ota_writer_finish()` gọi `esp_ota_end(ctx->handle)`. |
| Dùng `esp_ota_set_boot_partition()` | `ota_writer_finish()` gọi `esp_ota_set_boot_partition(update_partition)` nếu `set_boot_partition` true. |

## Yêu cầu dual-bank flash

| Yêu cầu | Implementation hiện tại |
| --- | --- |
| Có hai app slot OTA | `partitions.csv` có `ota_0` và `ota_1`, mỗi slot size `0x1F0000`. |
| Firmware mới ghi vào slot không hoạt động | `ota_writer_begin()` dùng `esp_ota_get_next_update_partition(NULL)`. |
| Bootloader boot vào image mới sau khi verify | `esp_ota_set_boot_partition()` cập nhật `otadata`, sau đó firmware gọi `esp_restart()`. |

## Yêu cầu rollback

| Yêu cầu | Implementation hiện tại |
| --- | --- |
| Bật rollback của ESP-IDF | `sdkconfig` có `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y` và `CONFIG_APP_ROLLBACK_ENABLE=y`. |
| Dùng NVS để lưu bộ đếm số lần cập nhật | `nvs_store.c` lưu `pending` và `boot_cnt` trong namespace `ota_store`. |
| Reset hơn 3 lần chưa confirm thì rollback | `ota_boot_health_check()` tăng boot counter và gọi rollback khi count > `NVS_STORE_MAX_OTA_BOOT_ATTEMPTS`. |
| Xác nhận firmware mới hợp lệ | `ota_confirm_running_app_if_pending()` gọi `ota_writer_mark_running_valid()` rồi clear NVS pending. |

## Yêu cầu UART interrupt-driven

| Yêu cầu | Implementation hiện tại |
| --- | --- |
| Nhận UART qua interrupt-driven queue | `uart_driver_install()` tạo `s_uart_event_queue`; receive path dùng `xQueueReceive()`. |
| Không polling thanh ghi trạng thái UART | Code không đọc thanh ghi UART trực tiếp; dùng ESP-IDF UART driver API. |
| Có timeout khi chờ packet | `ota_config.packet_timeout_ticks = pdMS_TO_TICKS(OTA_PACKET_TIMEOUT_MS)`. |

## Yêu cầu script Python host

| Yêu cầu | Implementation hiện tại |
| --- | --- |
| Đọc firmware binary | `Path(args.file).read_bytes()` trong `tools/ota_uart_send.py`. |
| Chia payload thành packet | Vòng `for offset in range(0, len(image), args.chunk_size)`. |
| Retry tối đa 3 lần mỗi packet | `--retries` mặc định 3; `send_frame_with_retry()` retry khi timeout hoặc NACK. |
| Có progress bar | `print_progress(sent, len(image))`. |
| Tính CRC32 image | `zlib.crc32(image) & 0xFFFFFFFF`. |
| Tính CRC16 packet | `crc16_ccitt()` trong script Python. |
| Có script demo dễ chạy | `tools/ota_test.sh`, `tools/ota_full.sh`, `tools/build_ota_test.sh` và `tools/flash_initial.sh` gom các lệnh dài thành lệnh ngắn. |

## Yêu cầu log trạng thái

| Giai đoạn | Log hiện tại |
| --- | --- |
| START | `START OTA: image_size=... expected_crc32=...` |
| WRITING | `WRITING: .../... bytes (...%)` |
| VERIFY | `VERIFY: calculating firmware CRC32` |
| SUCCESS | `SUCCESS: OTA image verified and boot partition updated` |
| FAIL | Các nhánh lỗi trong `ota_controller.c` và `ota_writer.c` log bằng `ESP_LOGE`. |
| REBOOT | `REBOOT: restarting after successful OTA` và `Restarting to boot selected OTA partition`. |

## Source test OTA

Firmware test OTA tối giản nằm ở `main/app/ota_test_app.c`. Mode này không thay thế firmware chính, mà là một build mode riêng để chứng minh OTA đổi version và boot sang partition mới.
