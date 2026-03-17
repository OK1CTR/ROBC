/**
 * @file       main.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      RadioOBC main module
 *
 * @addtogroup grMain
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <main.h>
#include <iwdg.h>
#include <systick.h>
#include <rtc.h>
#include <serial.h>
#include <stm32f100_msp.h>
#include <stdio.h>
#include <i2c.h>
#include <flag.h>

/* Definitions ---------------------------------------------------------------*/

//! Medium rate task period in SysTick cycles
#define RUN_PERIOD_MEDIUM              tick_ms(1000)

/* Functions -----------------------------------------------------------------*/

/* The application entry point */
int main(void)
{
    ticks_t tmr_run_medium;
    int cnt = 1;

    // MCU system initialization
    msp_init();
    iwdg_init();
    gpio_init();

    // modules initialization/
    flag_init();
    tick_init();
    rtc_init();
    serial_init();
    i2c_init();

    // local settings
    tick_timer_set(&tmr_run_medium, RUN_PERIOD_MEDIUM);
    printf("Start\n");

    // infinite loop
    while (1)
    {
        serial_job();

        if (tick_timer_expired(&tmr_run_medium))
        {
            tick_timer_set(&tmr_run_medium, RUN_PERIOD_MEDIUM);
            printf("Tick %d\n", cnt++);
            i2c_write(I2C_LED_BAR, cnt & 0xFF);
            iwdg_hit();
        }
    }
}


#if 0  /* reserved code */
/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
  /**ADC1 GPIO Configuration
  PA0-WKUP   ------> ADC1_IN0
  PA4   ------> ADC1_IN4
  PA5   ------> ADC1_IN5
  PA6   ------> ADC1_IN6
  PA7   ------> ADC1_IN7
  PB0   ------> ADC1_IN8
  PB1   ------> ADC1_IN9
  */
  GPIO_InitStruct.Pin = ADC0_Pin|ADC4_Pin|ADC5_Pin|ADC6_Pin
                          |TMP_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ADC8_Pin|ADC8B1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  LL_ADC_Init(ADC1, &ADC_InitStruct);
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
  LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_0);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_0, LL_ADC_SAMPLINGTIME_1CYCLE_5);
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{
  LL_SPI_InitTypeDef SPI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
  /**SPI1 GPIO Configuration
  PA15   ------> SPI1_NSS
  PB3   ------> SPI1_SCK
  PB4   ------> SPI1_MISO
  PB5   ------> SPI1_MOSI
  */
  GPIO_InitStruct.Pin = MNSS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(MNSS_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MSCK_Pin|MMOSI_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = MMISO_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(MMISO_GPIO_Port, &GPIO_InitStruct);

  LL_GPIO_AF_EnableRemap_SPI1();

  /* SPI1 parameter configuration*/
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
#endif

/* ---------------------------------------------------------------------------*/

/** @} */
