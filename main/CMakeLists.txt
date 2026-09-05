option(APP_OTA_TEST_MODE "Build minimal OTA validation firmware" OFF)
set(APP_OTA_TEST_VERSION "ota-test" CACHE STRING "Version label printed by OTA test firmware")

set(PROJECT_SRCS
    "main.c"
    "ota/ota_writer.c"
    "ota/ota_controller.c"
    "ota/nvs_store.c"
    "ota/crc_until.c"
    "ota/uart_proto.c"
)

if(APP_OTA_TEST_MODE)
    list(APPEND PROJECT_SRCS "app/ota_test_app.c")
else()
    list(APPEND PROJECT_SRCS
        "app/glucose_monitor.c"
        "drivers/max30102.c"
        "drivers/ssd1306.c"
    )
endif()

idf_component_register(
    SRCS ${PROJECT_SRCS}
    INCLUDE_DIRS "."
                 "app"
                 "drivers"
                 "ota"
                 "crypto"
                 "model"
    REQUIRES nvs_flash esp_wifi esp_event mqtt driver esp_netif esp_driver_gpio esp_driver_ledc esp_driver_i2c esp_driver_uart app_update
)

if(APP_OTA_TEST_MODE)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        APP_OTA_TEST_MODE=1
        APP_OTA_TEST_VERSION="${APP_OTA_TEST_VERSION}"
    )
else()
    target_compile_definitions(${COMPONENT_LIB} PRIVATE APP_OTA_TEST_MODE=0)
    target_link_libraries(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/crypto/libtinyjambu_static.a")
endif()
