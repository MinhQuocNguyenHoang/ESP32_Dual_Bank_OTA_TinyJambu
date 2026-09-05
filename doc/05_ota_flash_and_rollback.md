# OTA flash, dual-bank và rollback

Phần OTA của project dựa trên cơ chế OTA chính thức của ESP-IDF. Firmware không ghi trực tiếp vào địa chỉ flash thô. Mọi thao tác ghi app image đều đi qua `esp_ota_begin()`, `esp_ota_write()`, `esp_ota_end()` và `esp_ota_set_boot_partition()`.

## Partition table

Project dùng partition table custom trong `partitions.csv`:

```text
# Name,    Type, SubType, Offset,    Size
nvs,       data, nvs,     0x9000,    0x4000
otadata,   data, ota,     0xd000,    0x2000
phy_init,  data, phy,     0xf000,    0x1000
ota_0,     app,  ota_0,   0x10000,   0x1F0000
ota_1,     app,  ota_1,   0x200000,  0x1F0000
```

`ota_0` và `ota_1` là hai slot application. Tại một thời điểm, ESP32 đang chạy từ một slot. Khi cập nhật OTA, firmware mới được ghi vào slot còn lại. Sau khi ghi và verify thành công, bootloader được yêu cầu boot từ slot mới ở lần khởi động tiếp theo.

Với binary hiện tại khoảng 980 KB, mỗi slot `0x1F0000` byte đủ chứa firmware. Nếu binary lớn hơn kích thước slot, `ota_writer_begin()` sẽ trả lỗi `ESP_ERR_INVALID_SIZE` và OTA không bắt đầu.

## Vai trò của `otadata`

Partition `otadata` là vùng dữ liệu mà bootloader dùng để biết app partition nào nên được boot. Khi firmware gọi `esp_ota_set_boot_partition(update_partition)`, ESP-IDF cập nhật metadata trong vùng `otadata`. Sau khi reset, bootloader đọc metadata này và chọn partition mới.

Host Python không cần biết địa chỉ `ota_0` hay `ota_1`. Địa chỉ ghi thực tế được firmware chọn bằng:

```c
esp_ota_get_next_update_partition(NULL)
```

Nếu firmware đang chạy từ `ota_0`, partition update thường là `ota_1`. Nếu firmware đang chạy từ `ota_1`, partition update thường là `ota_0`. Vì vậy câu trả lời kỹ thuật cho “nạp vào địa chỉ nào” là: host gửi file qua UART, còn ESP32 tự chọn inactive OTA slot bằng ESP-IDF API.

## OTA writer

`ota_writer.c` là wrapper mỏng quanh ESP-IDF OTA API. Module này quản lý context:

```text
update_partition
handle
image_size
bytes_written
active
```

`ota_writer_begin()` nhận `image_size`, tìm partition update kế tiếp, kiểm tra kích thước image không vượt partition, rồi gọi `esp_ota_begin()`. Nếu thành công, writer context chuyển sang active.

`ota_writer_write()` ghi từng chunk firmware theo đúng thứ tự nhận được. Hàm này kiểm tra context đang active, data hợp lệ và tổng số byte ghi không vượt image size kỳ vọng.

`ota_writer_finish()` kiểm tra số byte đã ghi bằng đúng image size, gọi `esp_ota_end()` để ESP-IDF kiểm tra image format, rồi gọi `esp_ota_set_boot_partition()` nếu được yêu cầu. Sau đó context được clear.

`ota_writer_abort()` hủy session đang active bằng `esp_ota_abort()`. Hàm này được dùng khi host gửi ABORT hoặc khi controller phát hiện lỗi CRC32 ở cuối quá trình truyền.

## OTA controller

`ota_controller.c` là state machine chính của quá trình cập nhật. Controller có các trạng thái:

| State | Ý nghĩa |
| --- | --- |
| `IDLE` | Chưa có session OTA active. |
| `RECEIVING` | Đang nhận và ghi firmware chunks. |
| `VERIFYING` | Đã nhận END, đang tính và so sánh CRC32. |
| `SUCCESS` | Image đã verify, boot partition đã được set. |
| `FAILED` | Session lỗi. |

Khi nhận `START_OTA`, controller đọc `image_size` và `expected_crc32` từ payload. Sau đó gọi `ota_writer_begin()`, reset bộ đếm byte, khởi tạo CRC32 streaming và đặt `expected_seq = seq + 1`.

Khi nhận `DATA`, controller kiểm tra đang ở trạng thái `RECEIVING`, sequence đúng, payload không rỗng và tổng bytes không vượt image size. Nếu hợp lệ, payload được ghi bằng `ota_writer_write()` và được đưa vào `crc_util_crc32_update()`.

Khi nhận `END_OTA`, controller kiểm tra payload length bằng 0 và tổng số byte nhận đúng bằng image size. Sau đó controller gọi `crc_util_crc32_finish()` và so sánh với CRC32 host đã gửi trong START. Nếu khớp, controller set NVS pending flag, finish writer, set boot partition và reboot.

Khi nhận `ABORT`, controller hủy writer nếu session đang active, reset session metadata và quay về idle nếu abort thành công.

## Rollback bằng ESP-IDF và NVS

Rollback có hai phần: trạng thái OTA của ESP-IDF và bookkeeping riêng trong NVS.

ESP-IDF rollback được bật trong `sdkconfig`:

```text
CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y
CONFIG_APP_ROLLBACK_ENABLE=y
```

Project lưu thêm hai key trong namespace NVS `ota_store`:

```text
pending  -> firmware mới đang chờ xác nhận
boot_cnt -> số lần boot khi còn pending
```

Giới hạn hiện tại:

```text
NVS_STORE_MAX_OTA_BOOT_ATTEMPTS = 3
```

Trước khi set boot partition mới, controller gọi `nvs_store_set_ota_pending()`. Hàm này đặt `pending = 1` và reset `boot_cnt = 0`.

Khi firmware mới boot lên, `app_main()` gọi `ota_boot_health_check()`. Nếu firmware đang pending, boot counter tăng lên. Nếu counter lớn hơn 3, firmware gọi:

```c
esp_ota_mark_app_invalid_rollback_and_reboot()
```

API này đánh dấu app hiện tại là invalid và reboot để bootloader quay về image trước đó.

Nếu firmware boot ổn và đi qua `glucose_monitor_init()`, `app_main()` gọi `ota_confirm_running_app_if_pending()`. Hàm này gọi:

```c
esp_ota_mark_app_valid_cancel_rollback()
```

Sau đó pending flag và boot counter trong NVS được clear.

## Ý nghĩa của pending validation

Image OTA mới không nên được xem là tốt ngay sau khi ghi flash thành công. Ghi flash và CRC32 chỉ chứng minh file truyền không lỗi. Nó chưa chứng minh firmware mới boot được, init được phần cứng, chạy được logic chính hoặc kết nối được các subsystem quan trọng.

Vì vậy project xác nhận image mới sau khi firmware đã boot và đi qua quá trình init chính. Đây là self-test đơn giản. Nếu muốn chặt hơn, có thể thêm điều kiện như sensor init thành công, OLED init thành công, WiFi/MQTT không bắt buộc nhưng không crash, hoặc một health flag riêng từ state machine.

## Các tình huống lỗi và cách xử lý

Nếu image size lớn hơn OTA slot, `ota_writer_begin()` fail và controller trả NACK. Host dừng sau số lần retry.

Nếu mất điện giữa lúc đang ghi, boot partition chưa bị đổi. ESP32 vẫn boot firmware cũ.

Nếu mất điện sau khi set boot partition nhưng trước khi app mới confirm valid, bootloader và NVS pending counter sẽ bảo vệ bằng rollback sau các lần boot thất bại.

Nếu CRC32 cuối không khớp, controller abort session, không gọi `esp_ota_set_boot_partition()`, do đó firmware cũ vẫn được giữ.

Nếu firmware mới boot nhưng crash trước khi confirm, boot counter sẽ tăng sau mỗi lần boot. Sau khi vượt quá giới hạn, rollback API được gọi để quay về image cũ.
