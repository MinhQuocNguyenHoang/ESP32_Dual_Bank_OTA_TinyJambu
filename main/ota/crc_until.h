/**
 * @file crc_util.h
 * @brief CRC utility interface for UART OTA integrity verification.
 * @details Provides fixed-width integer CRC16 and CRC32 APIs used by UART_PROTO
 *          and OTA_CONTROLLER layers.
 */

#ifndef __CRC_UNTIL_H__
#define __CRC_UNTIL_H__

#include <stdint.h>

/**
 * @brief Calculates CRC-16/CCITT-FALSE for a contiguous data buffer.
 * @details Uses polynomial 0x1021, initial value 0xFFFF, no input/output reflection,
 *          and no final XOR. Intended for UART OTA packet integrity checks.
 * @param[in] data Pointer to input data buffer.
 * @param[in] len Number of bytes to process.
 * @return uint16_t Calculated CRC16 value.
 */
uint16_t crc_util_crc16_ccitt(const uint8_t *data, uint32_t len);

/**
 * @brief Initializes CRC32 streaming state.
 * @details Uses the standard reflected CRC32 initial value 0xFFFFFFFF.
 * @param None
 * @return uint32_t Initial CRC32 accumulator value.
 */
uint32_t crc_util_crc32_init(void);

/**
 * @brief Updates a CRC32 streaming accumulator with a new data chunk.
 * @details Uses the standard reflected CRC32 polynomial 0xEDB88320.
 *          This function does not apply the final XOR.
 * @param[in] crc Current CRC32 accumulator.
 * @param[in] data Pointer to input data chunk.
 * @param[in] len Number of bytes to process.
 * @return uint32_t Updated CRC32 accumulator.
 */
uint32_t crc_util_crc32_update(uint32_t crc, const uint8_t *data, uint32_t len);

/**
 * @brief Finalizes CRC32 streaming calculation.
 * @details Applies the standard CRC32 final XOR value 0xFFFFFFFF.
 * @param[in] crc Current CRC32 accumulator after all update calls.
 * @return uint32_t Final CRC32 checksum value.
 */
uint32_t crc_util_crc32_finish(uint32_t crc);

#endif // __CRC_UNTIL_H__
