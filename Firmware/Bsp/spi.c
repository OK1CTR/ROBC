/**
 * @file       spi.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      SPI communication module
 *
 * @addtogroup grSPI
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <spi.h>
#include <main.h>

/* Functions -----------------------------------------------------------------*/

/* Initialize SPI1 */
void spi1_init()
{
    LL_SPI_InitTypeDef SPI_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

    // SCK, MOSI
    GPIO_InitStruct.Pin = MSCK_Pin | MMOSI_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // MISO
    GPIO_InitStruct.Pin = MMISO_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(MMISO_GPIO_Port, &GPIO_InitStruct);

    LL_GPIO_AF_EnableRemap_SPI1();

    // SPI1 parameter configuration
    // TODO Move constants to the BSP
    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
    SPI_InitStruct.NSS = LL_SPI_NSS_HARD_INPUT;
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly = 10;
    LL_SPI_Init(SPI1, &SPI_InitStruct);
}


/* SPI1 transfer of 8 bits */
uint8_t spi1_trx8(uint8_t data_tx)
{
    LL_SPI_TransmitData8(SPI1, data_tx);
    while (!LL_SPI_IsActiveFlag_TXE(SPI1))
    {
    }
    while (!LL_SPI_IsActiveFlag_RXNE(SPI1))
    {
    }
    return LL_SPI_ReceiveData8(SPI1);
}

/* ---------------------------------------------------------------------------*/

/** @} */
