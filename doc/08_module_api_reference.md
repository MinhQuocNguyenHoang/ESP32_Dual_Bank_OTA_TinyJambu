# Module API Reference

This file summarizes the main internal firmware APIs. It is intended as a quick reference when reading the code or explaining the project during a review.

## `crc_until`

Header: `main/ota/crc_until.h`

`crc_util_crc16_ccitt(const uint8_t *data, uint32_t len)` calculates CRC16/CCITT-FALSE for a contiguous buffer. The UART protocol layer uses it to validate packet integrity.

`crc_util_crc32_init(void)` returns the initial CRC32 accumulator value, currently `0xFFFFFFFF`.

`crc_util_crc32_update(uint32_t crc, const uint8_t *data, uint32_t len)` updates a CRC32 accumulator with one data chunk. The OTA controller calls it after each DATA payload has been written to flash.

`crc_util_crc32_finish(uint32_t crc)` applies the final XOR and returns the final CRC32 checksum. The OTA controller compares this value against the CRC32 sent by the host in START_OTA.

## `uart_proto`

Header: `main/ota/uart_proto.h`

`uart_proto_init(const uart_proto_config_t *config)` configures the UART driver, sets TX/RX pins, installs the UART driver, and creates the event queue. The receive path is based on the ESP-IDF UART event queue.

`uart_proto_receive_packet(uart_proto_packet_t *packet, TickType_t timeout_ticks)` waits for one UART packet, searches for magic bytes, reads the header and payload, validates CRC16, and returns a decoded packet only when the frame is valid.

`uart_proto_send_ack(uint16_t seq, uart_proto_status_t status)` sends an ACK response frame for a sequence number.

`uart_proto_send_nack(uint16_t seq, uart_proto_status_t status)` sends a NACK response frame for a sequence number.

`uart_proto_deinit(void)` deletes the UART driver and clears protocol initialization state.

## `ota_controller`

Header: `main/ota/ota_controller.h`

`ota_controller_init(const ota_controller_config_t *config)` resets controller context, stores runtime configuration, and initializes the UART protocol layer.

`ota_controller_process_once(void)` processes one OTA packet. It receives a packet through `uart_proto`, dispatches START/DATA/END/ABORT handling, sends ACK/NACK, and restarts the device after a successful END_OTA if reboot is enabled.

`ota_controller_get_state(void)` returns the current OTA controller state. This can be used for debugging or future OLED status output.

`ota_controller_deinit(void)` aborts the current session if needed, deinitializes UART protocol, and clears controller context.

## `ota_writer`

Header: `main/ota/ota_writer.h`

`ota_writer_begin(ota_writer_context_t *ctx, uint32_t image_size)` finds the inactive OTA partition, validates the image size, calls `esp_ota_begin()`, and starts an OTA write session.

`ota_writer_write(ota_writer_context_t *ctx, const uint8_t *data, uint32_t len)` writes the next firmware chunk through `esp_ota_write()` and updates the written byte count.

`ota_writer_finish(ota_writer_context_t *ctx, bool set_boot_partition)` ends the write session with `esp_ota_end()`. If `set_boot_partition` is true, it calls `esp_ota_set_boot_partition()`.

`ota_writer_abort(ota_writer_context_t *ctx)` cancels an active OTA write session with `esp_ota_abort()`.

`ota_writer_mark_running_valid(void)` calls `esp_ota_mark_app_valid_cancel_rollback()` to confirm that the running image is valid.

`ota_writer_mark_running_invalid_and_rollback(void)` calls `esp_ota_mark_app_invalid_rollback_and_reboot()` to mark the current image invalid and roll back.

`ota_writer_restart(void)` calls `esp_restart()` after a successful OTA update.

## `nvs_store`

Header: `main/ota/nvs_store.h`

`nvs_store_set_ota_pending(void)` sets the pending flag in NVS and resets the OTA boot counter.

`nvs_store_clear_ota_pending(void)` clears the pending flag and resets the OTA boot counter.

`nvs_store_is_ota_pending(bool *pending)` reads the pending flag from NVS. If the key does not exist, the default value is false.

`nvs_store_get_ota_boot_count(uint32_t *count)` reads the OTA boot counter. If the key does not exist, the default value is zero.

`nvs_store_increment_ota_boot_count(uint32_t *count)` increments the OTA boot counter and optionally returns the updated value.

`nvs_store_reset_ota_boot_count(void)` resets the OTA boot counter to zero.

## `glucose_monitor`

Header: `main/app/glucose_monitor.h`

`glucose_monitor_init(void)` initializes the network stack, event loop, buzzer, MAX30102, SSD1306, WiFi, and MQTT. It also updates the OLED boot status screens.

`glucose_monitor_process(void)` runs one step of the glucose measurement state machine. It is called repeatedly from `app_main()`.

## `ota_test_app`

Header: `main/app/ota_test_app.h`

`ota_test_app_init(void)` logs the OTA test firmware version and the currently running partition. It is used when building with `APP_OTA_TEST_MODE=ON`.

`ota_test_app_process(void)` prints periodic heartbeat and free heap logs. It helps verify that the new image is running after OTA reboot.

## `max30102`

Header: `main/drivers/max30102.h`

`max30102_init(i2c_port_t i2c_num, gpio_num_t sda_pin, gpio_num_t scl_pin)` initializes the I2C driver and configures the MAX30102 sensor.

`max30102_check(void)` reads new sensor FIFO samples and stores them in the internal circular buffer.

`max30102_read_fifo(max30102_sample_t *sample)` updates the FIFO and returns the latest Red/IR sample.

`max30102_get_ir(uint32_t *ir_val)` updates the FIFO and returns the latest IR value for finger detection.

`max30102_reset_fifo(void)` resets the MAX30102 FIFO pointers.
