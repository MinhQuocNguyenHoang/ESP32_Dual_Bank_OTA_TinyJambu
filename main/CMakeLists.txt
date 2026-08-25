idf_component_register(
    SRCS "main.cpp"
         "glucose_monitor.cpp"
         "max30102.cpp"
         "ssd1306.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi esp_event mqtt driver esp_netif esp_driver_ledc esp_driver_i2c
)

# Link TinyJAMBU pre-compiled static library for ESP32
target_link_libraries(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libtinyjambu_static.a")
