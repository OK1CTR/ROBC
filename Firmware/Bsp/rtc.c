/**
 * @file       rtc.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Real time clock module
 *
 * @addtogroup grRtc
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <rtc.h>
#include <main.h>

/* Functions -----------------------------------------------------------------*/

/* RTC initialization */
extern void rtc_init()
{
#if 0
    LL_RTC_InitTypeDef RTC_InitStruct = {0};

    LL_PWR_EnableBkUpAccess();
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_BKP);
    LL_RCC_EnableRTC();

    RTC_InitStruct.AsynchPrescaler = 0xFFFFFFFFU;
    LL_RTC_Init(RTC, &RTC_InitStruct);
    LL_RTC_SetAsynchPrescaler(RTC, 0xFFFFFFFFU);
#endif
}

/* ---------------------------------------------------------------------------*/

/** @} */
