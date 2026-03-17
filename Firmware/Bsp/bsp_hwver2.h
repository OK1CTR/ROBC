/**
 * @file       bsp_hwver2.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      RadioOBC Board Specific Definitions - HW version 2 (first prototype)
 *
 * @addtogroup grBsp
 * @{
 */

#ifndef _BSP_HWVER2_H_
#define _BSP_HWVER2_H_

/* Includes ------------------------------------------------------------------*/

#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_iwdg.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_rtc.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_gpio.h"

/* Defines -------------------------------------------------------------------*/

/* USART1 */
#define TX1_Pin                      LL_GPIO_PIN_9
#define TX1_GPIO_Port                GPIOA
#define RX1_Pin                      LL_GPIO_PIN_10
#define RX1_GPIO_Port                GPIOA
#define DE1_Pin                      LL_GPIO_PIN_11
#define DE1_GPIO_Port                GPIOA

#define USART1_USART                 USART1
#define USART1_CLOCK_EN()            LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1)
#define USART1_BAUD_RATE             115200
#define USART1_IRQ_N                 USART1_IRQn
#define USART1_IRQ_HANDLER           USART1_IRQHandler
#define CONSOLE_SERIAL_1

/* USART2 */
#define DE2_Pin LL_GPIO_PIN_1
#define DE2_GPIO_Port GPIOA
#define TX2_Pin LL_GPIO_PIN_2
#define TX2_GPIO_Port GPIOA
#define RX2_Pin LL_GPIO_PIN_3
#define RX2_GPIO_Port GPIOA

/* ADC */
#define ADC0_Pin LL_GPIO_PIN_0
#define ADC0_GPIO_Port GPIOA
#define ADC4_Pin LL_GPIO_PIN_4
#define ADC4_GPIO_Port GPIOA
#define ADC5_Pin LL_GPIO_PIN_5
#define ADC5_GPIO_Port GPIOA
#define ADC6_Pin LL_GPIO_PIN_6
#define ADC6_GPIO_Port GPIOA
#define ADC8_Pin LL_GPIO_PIN_0
#define ADC8_GPIO_Port GPIOB
#define ADC8B1_Pin LL_GPIO_PIN_1
#define ADC8B1_GPIO_Port GPIOB

/* Temperature */
#define TMP_Pin LL_GPIO_PIN_7
#define TMP_GPIO_Port GPIOA

/* Power */
#define LOW_Pin LL_GPIO_PIN_12
#define LOW_GPIO_Port GPIOA

/* HW Watchdog */
#define WDI_Pin LL_GPIO_PIN_2
#define WDI_GPIO_Port GPIOB

/* Radio */
#define RMISO_Pin LL_GPIO_PIN_10
#define RMISO_GPIO_Port GPIOB
#define RMOSI_Pin LL_GPIO_PIN_11
#define RMOSI_GPIO_Port GPIOB
#define RSCK_Pin LL_GPIO_PIN_12
#define RSCK_GPIO_Port GPIOB
#define RDCL_Pin LL_GPIO_PIN_13
#define RDCL_GPIO_Port GPIOB
#define RDA_Pin LL_GPIO_PIN_14
#define RDA_GPIO_Port GPIOB
#define PAEN_Pin LL_GPIO_PIN_8
#define PAEN_GPIO_Port GPIOA
#define RSEL_Pin LL_GPIO_PIN_8
#define RSEL_GPIO_Port GPIOB
#define RIRQ_Pin LL_GPIO_PIN_9
#define RIRQ_GPIO_Port GPIOB

/* Debug */
#define SWDIO_Pin LL_GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin LL_GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA

/* Memory storage */
#define MNSS_Pin LL_GPIO_PIN_15
#define MNSS_GPIO_Port GPIOA
#define MSCK_Pin LL_GPIO_PIN_3
#define MSCK_GPIO_Port GPIOB
#define MMISO_Pin LL_GPIO_PIN_4
#define MMISO_GPIO_Port GPIOB
#define MMOSI_Pin LL_GPIO_PIN_5
#define MMOSI_GPIO_Port GPIOB

/* I2C */
#define SCL_Pin                      LL_GPIO_PIN_6
#define SCL_GPIO_Port                GPIOB
#define SDA_Pin                      LL_GPIO_PIN_7
#define SDA_GPIO_Port                GPIOB

#define I2C1_I2C                     I2C1
#define I2C1_CLOCK_EN()              LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1)
#define I2C1_CLOCK_SPEED             100000
#define I2C1_IRQ_N                   I2C1_EV_IRQn
#define I2C1_IRQ_HANDLER             I2C1_EV_IRQHandler
#define I2C1_ERROR_IRQ_N             I2C1_ER_IRQn
#define I2C1_ERROR_IRQ_HANDLER       I2C1_ER_IRQHandler

#define I2C_LED_BAR                  0x20


#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Generic GPIO initialization
 */
extern void gpio_init();

/* ---------------------------------------------------------------------------*/

#endif  /* _BSP_HWVER2_H_ */

/** @} */
