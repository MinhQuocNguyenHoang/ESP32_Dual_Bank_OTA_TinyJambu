/**
 * @file ota_test_app.h
 * @brief Minimal application used to validate UART OTA updates.
 */

#ifndef OTA_TEST_APP_H
#define OTA_TEST_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the OTA test application.
 * @param None
 * @return None
 */
void ota_test_app_init(void);

/**
 * @brief Runs one periodic OTA test application step.
 * @param None
 * @return None
 */
void ota_test_app_process(void);

#ifdef __cplusplus
}
#endif

#endif // OTA_TEST_APP_H
