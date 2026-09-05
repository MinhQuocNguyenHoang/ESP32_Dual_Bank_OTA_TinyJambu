/**
 * @file glucose_monitor.c
 * @brief Native ESP-IDF Implementation of Glucose Monitor FSM, Feature Extraction, TinyJAMBU Encryption & ESP-MQTT.
 * @details Conforms to embedded MISRA-C standards with strict fixed-width integer types and Doxygen documentation.
 */

#include "glucose_monitor.h"
#include "max30102.h"
#include "ssd1306.h"
#include "gradient_boosting_model.h"
#include "TinyJAMBU.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "driver/ledc.h"

/* ========================================================================= */
/*                          CONSTANTS AND DEFINITIONS                        */
/* ========================================================================= */

#define FINGER_THRESHOLD 30000L
#define SAMPLE_WINDOW 2000
#define SAMPLE_RATE 100
#define BUFFER_SIZE (SAMPLE_WINDOW / 1000 * SAMPLE_RATE)
#define STABILIZE_WINDOW_MS 5000

#define BUZZER_PIN GPIO_NUM_25

// Clinical Glucose Alert Thresholds
static const float HYPERGLYCEMIA_THRESHOLD = 180.0f; // mg/dL
static const float HYPOGLYCEMIA_THRESHOLD = 70.0f;   // mg/dL

// Network Credentials
static const char *WIFI_SSID = "Bill";
static const char *WIFI_PASS = "minhquoc2005";

// MQTT Gateway & Telemetry Configuration
static const char *MQTT_URI = "mqtt://192.168.1.120:1883";
static const char *MQTT_TOPIC_TELEMETRY = "medical/glucose_monitor/telemetry";
static const char *MQTT_TOPIC_ACK = "medical/glucose_monitor/ack";
static const char *MQTT_TOPIC_ACK_ALIAS = "node/sensor_phong_khach/ack";

static const char *TAG = "GLUCOSE_MONITOR_IDF";

// TinyJAMBU-128 Cryptographic Keys (NIST LWC)
static const uint8_t secret_key[16] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

static const uint8_t nonce[12] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B};

typedef struct
{
    uint8_t ciphertext[64];
    size_t length;
} encrypted_payload_t;

typedef enum
{
    STATE_IDLE,
    STATE_STABILIZING,
    STATE_SAMPLING,
    STATE_PREDICT,
    STATE_UPLOADING,
    STATE_WAIT_RELEASE
} display_state_t;

/* ========================================================================= */
/*                          STATIC GLOBAL VARIABLES                          */
/* ========================================================================= */

static display_state_t g_state = STATE_IDLE;
static display_state_t g_last_rendered_state = (display_state_t)-1;
static uint32_t g_stabilize_start_tick = 0;
static float g_ir_buffer[BUFFER_SIZE];
static float g_red_buffer[BUFFER_SIZE];
static uint32_t g_sample_timestamps[BUFFER_SIZE];
static uint32_t g_buffer_index = 0;
static float g_last_prediction = 0.0f;
static encrypted_payload_t g_last_encrypted;

static esp_mqtt_client_handle_t g_mqtt_client = NULL;
static bool s_wifi_connected = false;
static bool s_mqtt_connected = false;
static volatile bool s_web_ack_received = false;

/* ========================================================================= */
/*                          BUZZER CONTROLLER                                */
/* ========================================================================= */

/**
 * @brief Initializes the hardware PWM LEDC driver for passive buzzer on GPIO 25.
 * @param None
 * @return None
 */
static void buzzer_init(void)
{
    ledc_timer_config_t timer_conf = {};
    ledc_channel_config_t ch_conf = {};

    timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    timer_conf.duty_resolution = LEDC_TIMER_10_BIT;
    timer_conf.timer_num = LEDC_TIMER_0;
    timer_conf.freq_hz = 1000;
    timer_conf.clk_cfg = LEDC_AUTO_CLK;

    ch_conf.gpio_num = BUZZER_PIN;
    ch_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    ch_conf.channel = LEDC_CHANNEL_0;
    ch_conf.timer_sel = LEDC_TIMER_0;
    ch_conf.duty = 0;
    ch_conf.hpoint = 0;

    ledc_timer_config(&timer_conf);
    ledc_channel_config(&ch_conf);
}

/**
 * @brief Generates an audible tone on the buzzer at a specified frequency and duration.
 * @param[in] freq_hz Frequency of the tone in Hertz.
 * @param[in] duration_ms Duration of the tone in milliseconds.
 * @return None
 */
static void buzzer_tone(const uint32_t freq_hz, const uint32_t duration_ms)
{
    const TickType_t tone_ticks = pdMS_TO_TICKS(duration_ms);

    if (freq_hz > 0)
    {
        ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq_hz);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512); // 50% duty cycle
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(tone_ticks);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
}

/* ========================================================================= */
/*                          WIFI & MQTT HANDLERS                             */
/* ========================================================================= */

/**
 * @brief ESP-IDF Event handler callback for WiFi and IP network events.
 * @param[in] arg Event handler argument pointer.
 * @param[in] event_base Event base identifier.
 * @param[in] event_id Event identifier integer.
 * @param[in] event_data Pointer to event specific data structure.
 * @return None
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *ip_event = NULL;
    (void)arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "WiFi station started, connecting to SSID '%s'...", WIFI_SSID);
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        s_wifi_connected = false;
        ESP_LOGW(TAG, "WiFi disconnected, attempting reconnection in 2s...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "WiFi Connected! IP: " IPSTR, IP2STR(&ip_event->ip_info.ip));
        s_wifi_connected = true;

        if (g_mqtt_client != NULL && !s_mqtt_connected)
        {
            esp_mqtt_client_start(g_mqtt_client);
        }
    }
}

/**
 * @brief Initializes WiFi subsystem in Station (STA) mode.
 * @param None
 * @return None
 */
static void wifi_init_sta(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_event_handler_instance_t instance_any_id = NULL;
    esp_event_handler_instance_t instance_got_ip = NULL;
    wifi_config_t wifi_config = {};

    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/**
 * @brief Native ESP-MQTT event handler callback.
 * @param[in] handler_args User arguments pointer.
 * @param[in] base Event base identifier.
 * @param[in] event_id Event identifier integer.
 * @param[in] event_data Pointer to esp_mqtt_event_handle_t structure.
 * @return None
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    char topic[64] = {0};
    size_t t_len = 0;

    (void)handler_args;
    (void)base;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "[MQTT Connected] Connected to MQTT Broker Gateway!");
        s_mqtt_connected = true;
        esp_mqtt_client_subscribe(g_mqtt_client, MQTT_TOPIC_ACK, 0);
        esp_mqtt_client_subscribe(g_mqtt_client, MQTT_TOPIC_ACK_ALIAS, 0);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "[MQTT Disconnected] Connection to Broker lost.");
        s_mqtt_connected = false;
        break;

    case MQTT_EVENT_DATA:
        if (event->topic_len > 0)
        {
            t_len = (event->topic_len < (int32_t)(sizeof(topic) - 1)) ? (size_t)event->topic_len : (sizeof(topic) - 1);
            memcpy(topic, event->topic, t_len);
            topic[t_len] = '\0';
            if (strstr(topic, "ack") != NULL)
            {
                s_web_ack_received = true;
                ESP_LOGI(TAG, "[MQTT ACK Received] Web Gateway confirmed receipt on '%s'!", topic);
            }
        }
        break;

    default:
        break;
    }
}

/* ========================================================================= */
/*                          FEATURE EXTRACTION & ENCRYPTION                  */
/* ========================================================================= */

/**
 * @brief Extracts bio-spectral PPG features (Ratio, RMSSD, Waveform Slope).
 * @param[out] features Array buffer (length 3) to receive extracted feature values.
 * @return None
 */
static void extract_features(float *features)
{
    float ir_mean = 0.0f;
    float red_mean = 0.0f;
    float ir_var = 0.0f;
    float diff = 0.0f;
    float dt = 0.0f;
    uint32_t max_idx = 0;
    uint32_t min_idx = 0;
    float max_val = g_ir_buffer[0];
    float min_val = g_ir_buffer[0];
    uint32_t i = 0;

    for (i = 0; i < BUFFER_SIZE; i++)
    {
        ir_mean += g_ir_buffer[i];
        red_mean += g_red_buffer[i];
    }
    ir_mean /= (float)BUFFER_SIZE;
    red_mean /= (float)BUFFER_SIZE;
    features[0] = ir_mean / (red_mean + 0.000001f);

    for (i = 1; i < BUFFER_SIZE; i++)
    {
        diff = g_ir_buffer[i] - g_ir_buffer[i - 1];
        ir_var += diff * diff;
    }
    features[1] = sqrtf(ir_var / (float)(BUFFER_SIZE - 1));

    for (i = 1; i < BUFFER_SIZE; i++)
    {
        if (g_ir_buffer[i] > max_val)
        {
            max_val = g_ir_buffer[i];
            max_idx = i;
        }
        if (g_ir_buffer[i] < min_val)
        {
            min_val = g_ir_buffer[i];
            min_idx = i;
        }
    }

    if ((max_idx > min_idx) && ((max_idx - min_idx) > 5))
    {
        dt = (float)(g_sample_timestamps[max_idx] - g_sample_timestamps[min_idx]);
        features[2] = ((max_val - min_val) / (dt + 0.000001f)) * 1000.0f;
    }
    else
    {
        features[2] = 0.0f;
    }
}

/**
 * @brief Encrypts blood glucose measurement value using TinyJAMBU-128 AEAD algorithm.
 * @param[in] glucose_value Floating-point glucose prediction in mg/dL.
 * @param[out] payload Pointer to encrypted_payload_t structure to receive ciphertext.
 * @return None
 */
static void encrypt_glucose(const float glucose_value, encrypted_payload_t *payload)
{
    char plaintext[32] = {0};
    size_t mlen = 0;

    payload->length = 0;
    memset(payload->ciphertext, 0, sizeof(payload->ciphertext));

    snprintf(plaintext, sizeof(plaintext), "GLUCOSE:%.2f", glucose_value);
    mlen = strlen(plaintext);

    tinyjambu_128_aead_encrypt(
        payload->ciphertext, &payload->length,
        (const uint8_t *)plaintext, mlen,
        NULL, 0,
        nonce,
        secret_key);
}

/**
 * @brief Publishes encrypted telemetry payload to MQTT broker and waits for gateway ACK.
 * @param[in] encrypted Pointer to encrypted_payload_t structure.
 * @param[in] glucose_val Predicted glucose value for logging.
 * @return bool True if published and ACK received, false otherwise.
 */
static bool send_telemetry_mqtt(const encrypted_payload_t *encrypted, const float glucose_val)
{
    bool status = false;
    char hex_str[128] = {0};
    char payload[256] = {0};
    size_t i = 0;
    int32_t msg_id = 0;
    uint32_t start_time = 0;
    const TickType_t step_delay = pdMS_TO_TICKS(50);

    if (!s_wifi_connected)
    {
        ESP_LOGW(TAG, "[Upload Check] WiFi not connected -> Upload Failed");
        return false;
    }
    if (!s_mqtt_connected || g_mqtt_client == NULL)
    {
        ESP_LOGW(TAG, "[Upload Check] MQTT Broker not connected -> Upload Failed");
        return false;
    }

    s_web_ack_received = false;

    for (i = 0; i < encrypted->length; i++)
    {
        snprintf(hex_str + i * 2, 3, "%02X", encrypted->ciphertext[i]);
    }

    snprintf(payload, sizeof(payload), "{\"timestamp\":%lu,\"encrypted_hex\":\"%s\",\"glucose\":%.2f,\"length\":%u}",
             (unsigned long)(xTaskGetTickCount() * portTICK_PERIOD_MS), hex_str, glucose_val, (unsigned)encrypted->length);

    msg_id = esp_mqtt_client_publish(g_mqtt_client, MQTT_TOPIC_TELEMETRY, payload, 0, 0, 0);
    if (msg_id < 0)
    {
        ESP_LOGE(TAG, "[MQTT Error] Failed to publish encrypted telemetry payload!");
        return false;
    }

    ESP_LOGI(TAG, "[MQTT Sent Encrypted Data] %s", payload);

    // Wait up to 1500ms for ACK from Web Gateway Dashboard
    start_time = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
    while (((uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS) - start_time) < 1500)
    {
        if (s_web_ack_received)
        {
            ESP_LOGI(TAG, "[MQTT Success] Web Dashboard confirmed receipt!");
            status = true;
            break;
        }
        vTaskDelay(step_delay);
    }

    if (!status)
    {
        ESP_LOGW(TAG, "[MQTT Timeout] No ACK received from Web Gateway within 1.5s");
    }

    return status;
}

/* ========================================================================= */
/*                          INITIALIZATION ENTRY POINT                       */
/* ========================================================================= */

void glucose_monitor_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {};
    uint32_t timeout_count = 0;
    const TickType_t step_delay = pdMS_TO_TICKS(100);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    buzzer_init();
    max30102_init(I2C_NUM_0, GPIO_NUM_32, GPIO_NUM_33);
    ssd1306_init(I2C_NUM_0);

    wifi_init_sta();

    mqtt_cfg.broker.address.uri = MQTT_URI;
    g_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(g_mqtt_client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(g_mqtt_client);

    // Step A: Hardware Initialized
    ssd1306_clear();
    ssd1306_set_cursor(0, 0);
    ssd1306_set_text_size(1);
    ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
    ssd1306_println("  SYSTEM BOOTING...");
    ssd1306_println("1. Hardware: OK");
    ssd1306_println("2. WiFi: Connecting...");
    ssd1306_draw_footer();
    ssd1306_update();

    // Step B: Wait for WiFi Connection (Up to 10s)
    timeout_count = 0;
    while (!s_wifi_connected && (timeout_count < 100))
    {
        vTaskDelay(step_delay);
        timeout_count++;
    }

    ssd1306_clear();
    ssd1306_set_cursor(0, 0);
    ssd1306_set_text_size(1);
    ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
    ssd1306_println("  SYSTEM BOOTING...");
    ssd1306_println("1. Hardware: OK");
    if (s_wifi_connected)
    {
        ssd1306_println("2. WiFi: Connected!");
        ssd1306_println("3. MQTT: Connecting...");
    }
    else
    {
        ssd1306_println("2. WiFi: Timeout!");
        ssd1306_println("3. MQTT: Skipped");
    }
    ssd1306_draw_footer();
    ssd1306_update();

    // Step C: Wait for MQTT Broker Handshake (Up to 6s)
    if (s_wifi_connected)
    {
        timeout_count = 0;
        while (!s_mqtt_connected && (timeout_count < 60))
        {
            vTaskDelay(step_delay);
            timeout_count++;
        }

        ssd1306_clear();
        ssd1306_set_cursor(0, 0);
        ssd1306_set_text_size(1);
        ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
        ssd1306_println("  SYSTEM BOOTING...");
        ssd1306_println("1. Hardware: OK");
        ssd1306_println("2. WiFi: Connected!");
        if (s_mqtt_connected)
        {
            ssd1306_println("3. MQTT: Connected!");
        }
        else
        {
            ssd1306_println("3. MQTT: Timeout!");
        }
        ssd1306_draw_footer();
        ssd1306_update();
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(TAG, "Glucose Monitor ESP-IDF Platform Initialized");
}

/* ========================================================================= */
/*                          FSM STATE MACHINE PROCESS                        */
/* ========================================================================= */

void glucose_monitor_process(void)
{
    uint32_t current_ir = 0;
    int32_t progress = 0;
    float features[3] = {0.0f};
    float full_features[5] = {0.0f};
    bool upload_success = false;
    int32_t p = 0;
    max30102_sample_t sample = {};
    uint32_t elapsed_ms = 0;
    int32_t stab_progress = 0;
    float remaining_sec = 0.0f;

    max30102_get_ir(&current_ir);

    switch (g_state)
    {
    case STATE_IDLE:
        if (g_last_rendered_state != STATE_IDLE)
        {
            ssd1306_clear();
            ssd1306_set_text_size(1);
            ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
            ssd1306_set_cursor(18, 0);
            ssd1306_println("GLUCOSE MONITOR");
            ssd1306_set_cursor(16, 18);
            ssd1306_println("[ PLACE FINGER ]");
            ssd1306_set_cursor(25, 34);
            ssd1306_println("Status: Ready");
            ssd1306_draw_footer();
            ssd1306_update();
            g_last_rendered_state = STATE_IDLE;
        }

        g_buffer_index = 0;
        if (current_ir > FINGER_THRESHOLD)
        {
            g_state = STATE_STABILIZING;
            g_stabilize_start_tick = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
            g_last_rendered_state = (display_state_t)-1;
            ESP_LOGI(TAG, "[Auto Detect] Finger placed (IR=%lu). Stabilizing signal for %.1fs...", (unsigned long)current_ir, (float)STABILIZE_WINDOW_MS / 1000.0f);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        break;

    case STATE_STABILIZING:
        if (current_ir < FINGER_THRESHOLD)
        {
            g_state = STATE_IDLE;
            g_last_rendered_state = (display_state_t)-1;
            break;
        }

        max30102_check();
        elapsed_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS) - g_stabilize_start_tick;
        stab_progress = (int32_t)((elapsed_ms * 100) / STABILIZE_WINDOW_MS);
        if (stab_progress > 100)
        {
            stab_progress = 100;
        }
        remaining_sec = ((float)STABILIZE_WINDOW_MS - (float)elapsed_ms) / 1000.0f;
        if (remaining_sec < 0.0f)
        {
            remaining_sec = 0.0f;
        }

        ssd1306_clear();
        ssd1306_set_text_size(1);
        ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
        ssd1306_set_cursor(14, 0);
        ssd1306_println("-- STABILIZING --");
        ssd1306_set_cursor(16, 16);
        ssd1306_println("Hold Steady...");
        ssd1306_draw_progress_bar(10, 30, 108, 12, stab_progress);
        ssd1306_set_cursor(20, 45);
        ssd1306_printf("Measuring in %.1fs\n", remaining_sec);
        ssd1306_draw_footer();
        ssd1306_update();

        if (elapsed_ms >= STABILIZE_WINDOW_MS)
        {
            g_buffer_index = 0;
            g_state = STATE_SAMPLING;
            g_last_rendered_state = (display_state_t)-1;
            ESP_LOGI(TAG, "[Sensor] Signal stabilized! Starting PPG measurement...");
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        break;

    case STATE_SAMPLING:
        if (current_ir < FINGER_THRESHOLD)
        {
            ssd1306_clear();
            ssd1306_set_text_size(1);
            ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
            ssd1306_set_cursor(0, 15);
            ssd1306_println("  FINGER REMOVED!");
            ssd1306_set_cursor(0, 35);
            ssd1306_println(" Please place again");
            ssd1306_draw_footer();
            ssd1306_update();
            vTaskDelay(pdMS_TO_TICKS(1500));
            g_state = STATE_IDLE;
            g_last_rendered_state = (display_state_t)-1;
            g_buffer_index = 0;
            break;
        }

        if (g_buffer_index < BUFFER_SIZE)
        {
            max30102_read_fifo(&sample);
            g_ir_buffer[g_buffer_index] = (float)sample.ir;
            g_red_buffer[g_buffer_index] = (float)sample.red;
            g_sample_timestamps[g_buffer_index] = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
            g_buffer_index++;

            progress = (int32_t)((g_buffer_index * 100) / BUFFER_SIZE);

            // Update OLED progress bar periodically without saturating I2C bus
            if ((g_buffer_index % 10 == 0) || (g_buffer_index == BUFFER_SIZE))
            {
                ssd1306_clear();
                ssd1306_set_text_size(1);
                ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
                ssd1306_set_cursor(0, 0);
                ssd1306_println("-- PPG SAMPLING --");
                ssd1306_set_cursor(0, 16);
                ssd1306_printf("<3 Progress: %ld%%\n", (long)progress);
                ssd1306_draw_progress_bar(10, 30, 108, 12, progress);
                ssd1306_set_cursor(0, 45);
                ssd1306_println("Keep finger still...");
                ssd1306_draw_footer();
                ssd1306_update();
            }

            vTaskDelay(pdMS_TO_TICKS(1000 / SAMPLE_RATE));
        }
        else
        {
            g_state = STATE_PREDICT;
            g_last_rendered_state = (display_state_t)-1;
        }
        break;

    case STATE_PREDICT:
        ssd1306_clear();
        ssd1306_set_text_size(1);
        ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
        ssd1306_set_cursor(0, 0);
        ssd1306_println("  -- ANALYZING PPG --");
        ssd1306_set_cursor(0, 24);
        ssd1306_println("Running AI Model...");
        ssd1306_draw_footer();
        ssd1306_update();

        extract_features(features);
        full_features[0] = features[0];                             // Ratio
        full_features[1] = features[1];                             // RMSSD
        full_features[2] = features[2];                             // Slope
        full_features[3] = 60.0f / (features[1] + 0.000001f);       // Pulse rate
        full_features[4] = features[1] / (features[0] + 0.000001f); // AC/DC Ratio

        g_last_prediction = predict_gradient_boosting(full_features);

        ESP_LOGI(TAG, "--- AI PPG Feature Extraction ---");
        ESP_LOGI(TAG, "IR/Red Ratio: %.4f", full_features[0]);
        ESP_LOGI(TAG, "Variability (RMSSD): %.4f", full_features[1]);
        ESP_LOGI(TAG, "Slope: %.4f", full_features[2]);
        ESP_LOGI(TAG, "Pulse Rate: %.2f", full_features[3]);
        ESP_LOGI(TAG, "AC/DC Ratio: %.4f", full_features[4]);
        ESP_LOGI(TAG, "AI Model Prediction Result: %.2f mg/dL", g_last_prediction);
        ESP_LOGI(TAG, "---------------------------------");

        encrypt_glucose(g_last_prediction, &g_last_encrypted);

        ssd1306_clear();
        ssd1306_set_text_size(1);
        ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
        ssd1306_set_cursor(0, 0);
        ssd1306_println("== GLUCOSE RESULT ==");

        ssd1306_set_text_size(2);
        ssd1306_set_cursor(10, 18);
        ssd1306_printf("%.1f", g_last_prediction);
        ssd1306_set_text_size(1);
        ssd1306_println(" mg/dL");

        // Audio notification: Beep once to confirm measurement completion
        buzzer_tone(2000, 150);

        ssd1306_set_cursor(0, 40);
        if (g_last_prediction > HYPERGLYCEMIA_THRESHOLD)
        {
            ssd1306_println("[ HIGH GLUCOSE! ]");
            buzzer_tone(1000, 500);
        }
        else if (g_last_prediction < HYPOGLYCEMIA_THRESHOLD)
        {
            ssd1306_println("[ LOW GLUCOSE! ]");
            buzzer_tone(800, 500);
        }
        else
        {
            ssd1306_println("[ NORMAL GLUCOSE ]");
        }

        ssd1306_draw_footer();
        ssd1306_update();
        vTaskDelay(pdMS_TO_TICKS(3000));

        g_state = STATE_UPLOADING;
        g_last_rendered_state = (display_state_t)-1;
        break;

    case STATE_UPLOADING:
        ssd1306_clear();
        ssd1306_set_text_size(1);
        ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
        ssd1306_set_cursor(0, 0);
        ssd1306_println("-- SERVER UPLOAD --");
        ssd1306_set_cursor(0, 15);
        ssd1306_println("1. TinyJAMBU: OK");
        ssd1306_set_cursor(0, 27);
        ssd1306_println("2. Sending MQTT...");
        ssd1306_update();

        // Animate upload progress bar 0% -> 100%
        for (p = 0; p <= 100; p += 25)
        {
            ssd1306_clear();
            ssd1306_set_text_size(1);
            ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
            ssd1306_set_cursor(0, 0);
            ssd1306_println("-- SERVER UPLOAD --");
            ssd1306_set_cursor(0, 15);
            ssd1306_println("1. TinyJAMBU: OK");
            ssd1306_set_cursor(0, 27);
            ssd1306_println("2. Sending MQTT...");

            ssd1306_draw_progress_bar(10, 39, 108, 10, p);
            ssd1306_draw_footer();
            ssd1306_update();
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        upload_success = send_telemetry_mqtt(&g_last_encrypted, g_last_prediction);

        ssd1306_clear();
        ssd1306_set_text_size(1);
        ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
        ssd1306_set_cursor(0, 0);
        ssd1306_println("-- SERVER UPLOAD --");
        if (upload_success)
        {
            ssd1306_set_cursor(14, 20);
            ssd1306_println("== UPLOAD OK! ==");
            ssd1306_draw_progress_bar(10, 39, 108, 10, 100);
        }
        else
        {
            ssd1306_set_cursor(8, 20);
            ssd1306_println("== UPLOAD FAILED ==");
            ssd1306_draw_progress_bar(10, 39, 108, 10, 0);
        }
        ssd1306_draw_footer();
        ssd1306_update();
        vTaskDelay(pdMS_TO_TICKS(2000));

        g_state = STATE_WAIT_RELEASE;
        g_last_rendered_state = (display_state_t)-1;
        break;

    case STATE_WAIT_RELEASE:
        if (current_ir > FINGER_THRESHOLD)
        {
            ssd1306_clear();
            ssd1306_set_text_size(1);
            ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
            ssd1306_set_cursor(0, 15);
            ssd1306_println("   REMOVE FINGER...");
            ssd1306_set_cursor(0, 35);
            ssd1306_println("  Measure Complete!");
            ssd1306_draw_footer();
            ssd1306_update();
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        else
        {
            g_state = STATE_IDLE;
            g_last_rendered_state = (display_state_t)-1;
        }
        break;

    default:
        g_state = STATE_IDLE;
        g_last_rendered_state = (display_state_t)-1;
        break;
    }
}
