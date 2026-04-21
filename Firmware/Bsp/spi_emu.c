/**
 * @file       spi_emu.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Emulated SPI communication module
 *
 * @addtogroup grSPIemu
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <spi_emu.h>
#include <main.h>

/* Private macros ------------------------------------------------------------*/

//! MOSI line software control - H
#define set_mosi_H() LL_GPIO_SetOutputPin(MOSI_EMU_GPIO_Port, MOSI_EMU_Pin)
//! MOSI line software control - L
#define set_mosi_L() LL_GPIO_ResetOutputPin(MOSI_EMU_GPIO_Port, MOSI_EMU_Pin)
//! SCK line software control - H
#define set_sck_H() LL_GPIO_SetOutputPin(SCK_EMU_GPIO_Port, SCK_EMU_Pin)
//! SCK line software control - L
#define set_sck_L() LL_GPIO_ResetOutputPin(SCK_EMU_GPIO_Port, SCK_EMU_Pin)
//! Read the MISO line
#define get_miso() LL_GPIO_IsInputPinSet(MISO_EMU_GPIO_Port, MISO_EMU_Pin)

/* Functions -----------------------------------------------------------------*/

/* Initialize emulated SPI */
void spi_emu_init()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    // MOSI
    LL_GPIO_ResetOutputPin(MOSI_EMU_GPIO_Port, MOSI_EMU_Pin);
    GPIO_InitStruct.Pin = MOSI_EMU_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(MOSI_EMU_GPIO_Port, &GPIO_InitStruct);

    // MISO
    LL_GPIO_ResetOutputPin(SCK_EMU_GPIO_Port, SCK_EMU_Pin);
    GPIO_InitStruct.Pin = SCK_EMU_Pin;
    LL_GPIO_Init(SCK_EMU_GPIO_Port, &GPIO_InitStruct);

    // SCK
    GPIO_InitStruct.Pin = MISO_EMU_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    LL_GPIO_Init(MISO_EMU_GPIO_Port, &GPIO_InitStruct);
}


/* Emulated SPI transfer of 8 bits */
uint8_t spi_emu_trx8(uint8_t data_tx)
{
    uint32_t i, n;
    uint8_t data_rx;

    for (i = 0, n = 0x80, data_rx = 0; i < 8; i++, n >>= 1)
    {
        // transmit bit
        if (data_tx & n)
        {
            set_mosi_H();
        }
        else
        {
            set_mosi_L();
        }

        cycle_delay(5);

        // receive bit
        if (get_miso())
        {
            data_rx |= n;
        }

        // clock pulse
        set_sck_H();
        cycle_delay(5);
        set_sck_L();
        cycle_delay(5);
    }

    return data_rx;
}

/* ---------------------------------------------------------------------------*/

/** @} */
