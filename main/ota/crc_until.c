#include "crc_until.h"

#include <stddef.h>

uint16_t crc_util_crc16_ccitt(const uint8_t *data, uint32_t len)
{
    uint16_t crc = 0xFFFF;
    uint32_t i = 0U;
    uint8_t bit = 0;

    if (data != NULL)
    {
        for (i = 0U; i < len; i++)
        {
            crc = (uint16_t)(crc ^ ((uint16_t)data[i] << 8));

            for (bit = 0; bit < 8U; bit++)
            {
                if ((crc & 0x8000U) != 0U)
                {
                    crc = (uint16_t)((crc << 1) ^ 0x1021U);
                }
                else
                {
                    crc = (uint16_t)(crc << 1);
                }
            }
        }
    }

    return crc;
}

uint32_t crc_util_crc32_init(void)
{
    return 0xFFFFFFFF;
}

uint32_t crc_util_crc32_update(uint32_t crc, const uint8_t *data, uint32_t len)
{
    uint32_t i = 0U;
    uint8_t bit = 0;

    if (data != NULL)
    {
        for (i = 0U; i < len; i++)
        {
            crc = (uint32_t)(crc ^ (uint32_t)data[i]);

            for (bit = 0; bit < 8U; bit++)
            {
                if ((crc & 0x00000001U) != 0U)
                {
                    crc = (uint32_t)((crc >> 1) ^ 0xEDB88320U);
                }
                else
                {
                    crc = (uint32_t)(crc >> 1);
                }
            }
        }
    }

    return crc;
}

uint32_t crc_util_crc32_finish(uint32_t crc)
{
    return (uint32_t)(crc ^ 0xFFFFFFFFU);
}
