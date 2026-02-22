/*!
 * \addtogroup Backup Backup Domain
 * \brief Backup Domain low level driver
 * @{
 */

/*!
 * \file    pils_bkp.c
 * \brief   Backup Domain low level driver source
 * \author  OK1CTR
 * \version 1.0
 * \date    02.2020
 */


#include <stdint.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "pils_config.h"
#include "pils_bkp.h"


// Software RTC to sync with HW RTC using 1s interrupt
volatile uint32_t sw_rtc = 0;


/*!\brief RTC Interrupt Handler
 */
void RTC_IRQHandler(void)
{
	// second flag - 1s interrupt
	if (RTC->CRL & RTC_CRL_SECF) {
		sw_rtc++;
#if RTC_SYNC_DIV > 0
		// every n-th second check the HW clock
		if (!(sw_rtc % RTC_SYNC_DIV)) sw_rtc_sync();
#endif
		RTC->CRL &= ~RTC_CRL_SECF;
	}
	return;
}


/* Initialize the STM32 backup domain and RTC */
uint8_t bkp_init(void)
{
	uint16_t r;
	
	// enable APB clock and unlock the backup domain
	RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
	PWR->CR |= PWR_CR_DBP;
	// check the backup domain reset state
	if (RCC->BDCR == 0) r = 1; else r = 0;
	// init the RTC clock if after reset
	if (r) {
		RCC->BDCR |= RCC_BDCR_LSEON | RCC_BDCR_RTCEN | (1 << 8);
		while (!(RTC->CRL & RTC_CRL_RTOFF))
			;
		RTC->CRL |= RTC_CRL_CNF;
		// begin of config
		RTC->PRLH = 0;
		RTC->PRLL = 0x7FFF;
		RTC->CNTH = 0;
		RTC->CNTL = 0;
		// end of config
		RTC->CRL &= ~RTC_CRL_CNF;
		while (!(RTC->CRL & RTC_CRL_RTOFF))
			;
	}
	// 1s interrupt enable
	RTC->CRH |= RTC_CRH_SECIE;
	while (!(RTC->CRL & RTC_CRL_RTOFF))
		;
	// RTC interrupt enable
	NVIC_EnableIRQ(RTC_IRQn);	
	// lock the backup domain
	PWR->CR &= ~PWR_CR_DBP;
	return(r);
}


/* RTC time setting */
void rtc_set_time(uint32_t time)
{
	// unlock the backup domain
	PWR->CR |= PWR_CR_DBP;
	while (!(RTC->CRL & RTC_CRL_RTOFF))
		;
	// enable config
	RTC->CRL |= RTC_CRL_CNF;
	RTC->CNTH = time >> 16;
	RTC->CNTL = time & 0xFFFF;
	// disable config
	RTC->CRL &= ~RTC_CRL_CNF;
	while (!(RTC->CRL & RTC_CRL_RTOFF))
		;
	// lock the backup domain
	PWR->CR &= ~PWR_CR_DBP;
	return;
}


/* RTC alarm setting */
void rtc_set_alarm(uint32_t time)
{
	// unlock the backup domain
	PWR->CR |= PWR_CR_DBP;
	while (!(RTC->CRL & RTC_CRL_RTOFF))
		;
	// enable config
	RTC->CRL |= RTC_CRL_CNF;
	RTC->ALRH = time >> 16;
	RTC->ALRL = time & 0xFFFF;
	// disable config
	RTC->CRL &= ~RTC_CRL_CNF;
	while (!(RTC->CRL & RTC_CRL_RTOFF))
		;
	// lock the backup domain
	PWR->CR &= ~PWR_CR_DBP;
	return;
}


/* RTC time read */
uint32_t rtc_get_time(void)
{
	uint32_t time;

	// unlock the backup domain
	PWR->CR |= PWR_CR_DBP;
	time = (RTC->CNTH << 16) | RTC->CNTL;
	// lock the backup domain
	PWR->CR &= ~PWR_CR_DBP;
	return(time);
}


/* Backup domain register set */
void bkp_set_val(uint8_t adr, uint16_t data)
{
	// unlock the backup domain
	PWR->CR |= PWR_CR_DBP;
	switch (adr) {
		case 0: BKP->DR1  = data; break;
		case 1: BKP->DR2  = data; break;
		case 2: BKP->DR3  = data; break;
		case 3: BKP->DR4  = data; break;
		case 4: BKP->DR5  = data; break;
		case 5: BKP->DR6  = data; break;
		case 6: BKP->DR7  = data; break;
		case 7: BKP->DR8  = data; break;
		case 8: BKP->DR9  = data; break;
		case 9: BKP->DR10 = data; break;
	}
	// lock the backup domain
	PWR->CR &= ~PWR_CR_DBP;
	return;
}


/* Backup domain register read */
uint16_t bkp_get_val(uint8_t adr)
{
	uint16_t data;
	
	// unlock the backup domain
	PWR->CR |= PWR_CR_DBP;
	switch (adr) {
		case  0: data = BKP->DR1;  break;
		case  1: data = BKP->DR2;  break;
		case  2: data = BKP->DR3;  break;
		case  3: data = BKP->DR4;  break;
		case  4: data = BKP->DR5;  break;
		case  5: data = BKP->DR6;  break;
		case  6: data = BKP->DR7;  break;
		case  7: data = BKP->DR8;  break;
		case  8: data = BKP->DR9;  break;
		case  9: data = BKP->DR10; break;
	}	
	// lock the backup domain
	PWR->CR &= ~PWR_CR_DBP;
	return(data);
}


/*! @} */
