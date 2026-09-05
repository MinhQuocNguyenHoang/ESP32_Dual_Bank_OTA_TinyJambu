# Glucose Measurement Pipeline

`glucose_monitor.c` owns the non-invasive glucose measurement pipeline. The flow includes finger detection, signal stabilization, PPG sampling, feature extraction, model inference, result encryption, display output, MQTT telemetry, and gateway acknowledgment.

## Signal Acquisition Hardware

The MAX30102 sensor is used in SpO2 mode and provides Red and IR optical samples. The firmware communicates with it over I2C0.

```text
I2C port: I2C_NUM_0
SDA: GPIO32
SCL: GPIO33
MAX30102 address: 0x57
```

`max30102_init()` configures the ESP-IDF I2C master driver, resets the sensor, enables FIFO rollover, selects SpO2 mode, configures sample rate and pulse width, sets Red/IR LED current, and resets FIFO pointers.

`max30102_check()` reads the sensor FIFO write and read pointers to determine how many samples are available. Each sample has 6 bytes: 3 bytes for Red and 3 bytes for IR. The raw values are masked to 18 bits with `0x03FFFF`.

## Sample Buffers

The measurement window is configured as:

```text
SAMPLE_WINDOW = 2000 ms
SAMPLE_RATE   = 100 Hz
BUFFER_SIZE   = 200 samples
```

The firmware stores samples in:

```text
g_ir_buffer[BUFFER_SIZE]
g_red_buffer[BUFFER_SIZE]
g_sample_timestamps[BUFFER_SIZE]
```

During `STATE_SAMPLING`, every acquired sample stores Red, IR, and timestamp data. After 200 samples are collected, the state machine transitions to prediction.

## Finger Detection

Finger detection is based on the current IR value:

```text
FINGER_THRESHOLD = 30000
```

If `current_ir > FINGER_THRESHOLD`, the firmware treats the finger as present and moves from `STATE_IDLE` to `STATE_STABILIZING`. If IR drops below the threshold during stabilization or sampling, the firmware returns to idle.

This fixed threshold is simple and works for demonstration. A production system would likely need adaptive thresholding, filtering, and calibration for different ambient light conditions and users.

## Measurement State Machine

Full measurement flow:

```text
IDLE
  -> STABILIZING
  -> SAMPLING
  -> PREDICT
  -> UPLOADING
  -> WAIT_RELEASE
  -> IDLE
```

`STATE_IDLE` renders the idle screen and resets the sample index. When a finger is detected, the firmware records the stabilization start time.

`STATE_STABILIZING` runs for 5 seconds and displays progress on the OLED. The goal is to let the optical signal settle before collecting the measurement window.

`STATE_SAMPLING` records 200 Red/IR samples. OLED updates are throttled so the I2C bus is not saturated by display refreshes.

`STATE_PREDICT` extracts features, runs the model, encrypts the result, displays glucose on the OLED, and triggers buzzer feedback. Values above 180 mg/dL and below 70 mg/dL trigger alert tones.

`STATE_UPLOADING` sends encrypted telemetry through MQTT and waits up to 1.5 seconds for an ACK from the gateway.

`STATE_WAIT_RELEASE` waits until the user removes the finger before returning to idle.

## Feature Extraction

`extract_features()` produces three direct features from the Red/IR buffers.

| Feature | Calculation | Meaning |
| --- | --- | --- |
| `features[0]` | Mean IR divided by mean Red | Relative absorption ratio between optical channels. |
| `features[1]` | RMS of consecutive IR differences | PPG signal variability. |
| `features[2]` | Slope between valid min and max IR points | Approximate waveform rise rate. |

Two additional derived features are then created:

```text
full_features[3] = 60.0 / (features[1] + epsilon)
full_features[4] = features[1] / (features[0] + epsilon)
```

`epsilon = 0.000001` prevents division by zero.

The model is called with:

```c
predict_gradient_boosting(full_features)
```

The implementation is stored in `main/model/gradient_boosting_model.h`, generated from the Python training pipeline.

## TinyJAMBU Encryption

After prediction, the firmware creates plaintext in this form:

```text
GLUCOSE:<value>
```

It then calls TinyJAMBU-128 AEAD:

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

The current key is a 16-byte constant and the current nonce is a 12-byte constant in firmware. This is acceptable for a demonstration, but a real product should not reuse the same nonce with the same key. A better design would derive a unique nonce from a persistent counter, device ID, timestamp, or a combination of these values.

## MQTT Telemetry

The firmware publishes JSON telemetry to:

```text
medical/glucose_monitor/telemetry
```

Payload shape:

```json
{
  "timestamp": 123456,
  "encrypted_hex": "...",
  "glucose": 105.50,
  "length": 28
}
```

The firmware waits for ACK messages on:

```text
medical/glucose_monitor/ack
node/sensor_phong_khach/ack
```

If an MQTT message arrives on a topic containing `ack`, `s_web_ack_received` is set to true. `send_telemetry_mqtt()` waits up to 1.5 seconds for this condition.

## Python Dashboard

`app/app.py` runs a Flask and SocketIO dashboard. It subscribes to telemetry topics, reads `encrypted_hex`, decrypts the TinyJAMBU payload, derives the glucose value, classifies the result, stores the measurement in SQLite, and emits a WebSocket event for the web UI.

Classification thresholds:

```text
glucose > 180.0  -> Hyperglycemia
glucose < 70.0   -> Hypoglycemia
otherwise        -> Normal
```

After processing each telemetry message, the dashboard publishes ACK messages back to the ESP32.

## Current Limitations

The current measurement pipeline is suitable for demonstrating the end-to-end embedded architecture, but it is not a medical-grade implementation. Feature extraction is simple, finger detection uses a fixed threshold, the model depends on sample training data, and TinyJAMBU currently uses a fixed nonce.

Natural improvements include sensor calibration per user, PPG filtering before feature extraction, raw sample logging for model evaluation, safer key and nonce management, and moving WiFi/MQTT configuration out of source code.
