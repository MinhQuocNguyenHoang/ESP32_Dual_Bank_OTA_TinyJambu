/**
 * @file crc_util.h
 * @brief CRC utility interface for UART OTA integrity verification.
 * @details Provides fixed-width integer CRC16 and CRC32 APIs used by UART_PROTO
 *          and OTA_CONTROLLER layers.
 */

#ifndef __CRC_UNTIL_H__
#define __CRC_UNTIL_H__

uint16_t crc_util_crc16_ccitt(const uint8_t *data, uint32_t len);

uint32_t crc_util_crc32_init(void);
uint32_t crc_util_crc32_update(uint32_t crc, const uint8_t *data, uint32_t len);
uint32_t crc_util_crc32_finish(uint32_t crc);
uint32_t crc_util_crc32(const uint8_t *data, uint32_t len);

#endif // __CRC_UNTIL_H__