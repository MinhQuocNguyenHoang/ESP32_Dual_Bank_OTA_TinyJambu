# Pipeline đo glucose và truyền dữ liệu

Module `glucose_monitor.c` quản lý pipeline đo glucose không xâm lấn. Pipeline này gồm phát hiện ngón tay, ổn định tín hiệu, lấy mẫu PPG, trích xuất đặc trưng, suy luận mô hình, mã hóa kết quả và gửi telemetry qua MQTT.

## Phần cứng đo tín hiệu

Cảm biến MAX30102 được dùng ở chế độ SpO2, đọc hai kênh Red và IR. Firmware giao tiếp với cảm biến qua I2C0:

```text
I2C port: I2C_NUM_0
SDA: GPIO32
SCL: GPIO33
MAX30102 address: 0x57
```

Trong `max30102_init()`, firmware cấu hình I2C master 400 kHz, reset sensor, cấu hình FIFO rollover, chọn SpO2 mode, đặt sample rate và pulse width, cấu hình dòng LED Red/IR, sau đó reset FIFO pointer.

Driver `max30102_check()` đọc FIFO write pointer và read pointer để biết có bao nhiêu mẫu mới. Mỗi sample gồm 6 byte: 3 byte Red và 3 byte IR. Giá trị raw được mask về 18 bit bằng `0x03FFFF`.

## Bộ đệm mẫu

Trong `glucose_monitor.c`, cửa sổ lấy mẫu được cấu hình:

```text
SAMPLE_WINDOW = 2000 ms
SAMPLE_RATE   = 100 Hz
BUFFER_SIZE   = 200 samples
```

Firmware lưu ba buffer:

```text
g_ir_buffer[BUFFER_SIZE]
g_red_buffer[BUFFER_SIZE]
g_sample_timestamps[BUFFER_SIZE]
```

Mỗi lần đọc sample trong `STATE_SAMPLING`, firmware lưu Red, IR và timestamp hiện tại. Sau khi đủ 200 mẫu, state machine chuyển sang `STATE_PREDICT`.

## Phát hiện ngón tay

Firmware dùng IR raw để phát hiện ngón tay:

```text
FINGER_THRESHOLD = 30000
```

Nếu `current_ir > FINGER_THRESHOLD`, firmware xem như có ngón tay và chuyển từ `STATE_IDLE` sang `STATE_STABILIZING`. Nếu trong lúc ổn định hoặc lấy mẫu mà IR xuống dưới ngưỡng, firmware xem như người dùng đã rút tay và quay lại idle.

Đây là cách phát hiện đơn giản, phù hợp demo. Trong bản cải tiến, ngưỡng có thể được hiệu chỉnh theo môi trường, hoặc dùng trung bình trượt để tránh nhiễu làm nhảy state.

## State machine đo

Luồng đo đầy đủ:

```text
IDLE
  -> STABILIZING
  -> SAMPLING
  -> PREDICT
  -> UPLOADING
  -> WAIT_RELEASE
  -> IDLE
```

`STATE_IDLE` hiển thị màn hình chờ và reset index buffer. Khi phát hiện ngón tay, firmware ghi lại thời điểm bắt đầu ổn định.

`STATE_STABILIZING` chạy trong 5 giây. OLED hiển thị progress bar. Mục đích là để tín hiệu quang học bớt nhiễu do người dùng vừa đặt tay.

`STATE_SAMPLING` lấy 200 mẫu với chu kỳ logic `1000 / SAMPLE_RATE`. OLED chỉ cập nhật mỗi 10 mẫu hoặc khi hoàn tất để tránh dùng I2C quá dày.

`STATE_PREDICT` tính đặc trưng, chạy mô hình, mã hóa kết quả và hiển thị glucose lên OLED. Nếu glucose cao hơn 180 mg/dL hoặc thấp hơn 70 mg/dL, buzzer phát cảnh báo.

`STATE_UPLOADING` gửi telemetry qua MQTT. Firmware chờ ACK từ gateway trong khoảng 1.5 giây. OLED hiển thị upload OK hoặc failed.

`STATE_WAIT_RELEASE` yêu cầu người dùng rút tay trước khi đo lượt mới.

## Feature extraction

Hàm `extract_features()` tạo ba đặc trưng trực tiếp từ buffer Red/IR:

| Feature | Cách tính | Ý nghĩa |
| --- | --- | --- |
| `features[0]` | Mean IR chia Mean Red | Tỷ lệ hấp thụ tương đối giữa hai kênh quang học. |
| `features[1]` | RMS của sai phân liên tiếp trên IR | Độ biến thiên tín hiệu PPG. |
| `features[2]` | Độ dốc giữa điểm min và max hợp lệ | Đại diện cho tốc độ thay đổi waveform. |

Sau đó firmware tạo thêm hai feature phụ:

```text
full_features[3] = 60.0 / (features[1] + epsilon)
full_features[4] = features[1] / (features[0] + epsilon)
```

`epsilon = 0.000001` được dùng để tránh chia cho 0.

Mô hình được gọi bằng:

```c
predict_gradient_boosting(full_features)
```

Hàm này nằm trong `gradient_boosting_model.h`, là model đã được export từ pipeline huấn luyện phía `software/`.

## TinyJAMBU encryption

Sau khi có glucose prediction, firmware tạo plaintext dạng:

```text
GLUCOSE:<value>
```

Sau đó gọi TinyJAMBU-128 AEAD:

```c
tinyjambu_128_aead_encrypt(
    ciphertext,
    &length,
    plaintext,
    mlen,
    NULL,
    0,
    nonce,
    secret_key);
```

Key hiện tại là 16 byte hardcoded trong firmware. Nonce hiện tại là 12 byte hardcoded. Đây phù hợp cho demo kỹ thuật, nhưng trong sản phẩm thật không nên dùng nonce cố định cho nhiều message với cùng key. Nên dùng nonce duy nhất cho mỗi telemetry message, ví dụ sinh từ counter lưu NVS hoặc timestamp kết hợp device ID.

## MQTT telemetry

Firmware publish payload JSON lên topic:

```text
medical/glucose_monitor/telemetry
```

Payload hiện tại có dạng:

```json
{
  "timestamp": 123456,
  "encrypted_hex": "...",
  "glucose": 105.50,
  "length": 28
}
```

Sau khi publish, firmware chờ ACK từ một trong các topic:

```text
medical/glucose_monitor/ack
node/sensor_phong_khach/ack
```

Nếu nhận message có topic chứa chuỗi `ack`, biến `s_web_ack_received` được set true. Hàm `send_telemetry_mqtt()` chờ tối đa 1.5 giây. Nếu có ACK, upload được xem là thành công.

## Web dashboard

`app/app.py` chạy Flask và SocketIO. App subscribe MQTT telemetry, đọc `encrypted_hex`, giải mã TinyJAMBU bằng key và nonce tương ứng, suy ra glucose value, phân loại mức glucose, lưu SQLite và emit dữ liệu mới qua WebSocket cho frontend.

Các mức phân loại:

```text
glucose > 180.0  -> Hyperglycemia
glucose < 70.0   -> Hypoglycemia
ngược lại        -> Normal
```

Sau khi xử lý message, dashboard publish ACK ngược về ESP32 để hoàn tất vòng xác nhận telemetry.

## Giới hạn kỹ thuật hiện tại

Pipeline đo hiện tại phù hợp để demo xử lý tín hiệu và kiến trúc end-to-end, nhưng chưa phải thiết bị y tế hoàn chỉnh. Feature extraction còn đơn giản, ngưỡng phát hiện ngón tay cố định, model phụ thuộc vào dữ liệu huấn luyện mẫu, và TinyJAMBU đang dùng nonce cố định.

Các hướng cải tiến tự nhiên gồm hiệu chuẩn cảm biến theo từng người dùng, lọc nhiễu PPG trước khi lấy feature, lưu dữ liệu đo thô để đánh giá model, quản lý key/nonce an toàn hơn, và tách cấu hình WiFi/MQTT ra khỏi source code.
