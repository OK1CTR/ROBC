/*!
 * \addtogroup Backup Backup Domain
 * \brief Backup Domain low level driver
 * @{
 */

/*!
 * \file    pils_bkp.h
 * \brief   Backup Domain low level driver header
 * \author  OK1CTR
 * \version 1.0
 * \date    02.2020
 */


#ifndef _PILS_BKP_H_
#define _PILS_BKP_H_


//! Maximal address for backup domain register access
#define BKP_MAXADR 9
//! HW -> SW sync is performed every RTC_SYNC_DIV'th second. When 0, automatic sync disabled.
#define RTC_SYNC_DIV 60


//! SW and HW synchronization
#define sw_rtc_sync() sw_rtc = rtc_get_time()
//! Test the RTC alarm flag
#define rtc_alarm_flag() (RTC->CRL & RTC_CRL_ALRF)
//! Clear the RTC alarm flag
#define rtc_alarm_clear() RTC->CRL &= ~RTC_CRL_ALRF


//! Software RTC to sync with HW RTC using 1s interrupt
extern volatile uint32_t sw_rtc;


/*!\brief Initialize the STM32 backup domain and RTC
 * \return zero, if RTC still initialized, nonzero after a backup domain reset
 */
extern uint8_t bkp_init(void);

/*!\brief RTC time setting
 * \param time Number of second since Epoch to set
 */
extern void rtc_set_time(uint32_t time);

/*!\brief RTC alarm setting
 * \param time Number of second since Epoch to set
 */
extern void rtc_set_alarm(uint32_t time);

/*!\brief RTC time read
 * \return time Number of second since Epoch to set
 */
extern uint32_t rtc_get_time(void);

/*!\brief Backup domain register set
 * \param adr Register address, 0 to 20
 * \param data Data to set (16 bit)
 */
extern void bkp_set_val(uint8_t adr, uint16_t data);

/*!\brief Backup domain register read
 * \param adr Register address, 0 to BKP_MAXADR
 * \return Register value read (16 bit)
 */
extern uint16_t bkp_get_val(uint8_t adr);

#endif

/*! @} */
