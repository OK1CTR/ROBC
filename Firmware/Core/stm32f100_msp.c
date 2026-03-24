/**
 * @file       stm32f100_msp.c
 * @author     OK1CTR
 * @date       Mar 2026
 * @brief      MCU specific module for STM32F100
 *
 * @addtogroup grMsp
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <main.h>
#include <stm32f100_msp.h>

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief System clock configuration
 */
static void system_clock_config(void);

/* Functions -----------------------------------------------------------------*/

/* Initialize the MCU system */
void msp_init()
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    LL_GPIO_AF_Remap_SWJ_NOJTAG();
    system_clock_config();
    SystemCoreClockUpdate();

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
}

/* Private functions ---------------------------------------------------------*/

/* System clock configuration */
static void system_clock_config(void)
{
    // enable HSE
    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() != 1)
    {
    }

    // ebable LSI
    LL_RCC_LSI_Enable();
    while (LL_RCC_LSI_IsReady() != 1)
    {
    }

    // backup domain reset, if not LSE
    LL_PWR_EnableBkUpAccess();
    if (LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
    {
        LL_RCC_ForceBackupDomainReset();
        LL_RCC_ReleaseBackupDomainReset();
    }

    // enable LSE, set RTC on LSE
    LL_RCC_LSE_Enable();
    while (LL_RCC_LSE_IsReady() != 1)
    {
    }
    if (LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
    {
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    }
    LL_RCC_EnableRTC();

    // enable and configure PLL
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_3);
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1)
    {
    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    // set core clock
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
    {
    }

    // set ADC clock
    LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSRC_PCLK2_DIV_2);
}

/* ---------------------------------------------------------------------------*/

/** @} */
