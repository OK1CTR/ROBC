/**
 * @file       app.h
 * @author     OK1CTR
 * @date       May 2026
 * @brief      Common application module header
 *
 * @addtogroup grApplication
 * @{
 */

#ifndef _APP_H_
#define _APP_H_

/* Includes ------------------------------------------------------------------*/

#include <systick.h>
#include <critical.h>
#include <rtc.h>
#include <serial.h>
#include <stm32f100_msp.h>
#include <stdio.h>
#include <i2c.h>
#include <flag.h>
#include <watchdog.h>
#include <ax5043.h>
#include <radio.h>
#include <spi.h>
#include <spi_emu.h>
#include <morse.h>

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Application related initialization
 */
extern void app_init();

/**
 * @brief Application main body
 */
extern void app_run();

/* ---------------------------------------------------------------------------*/

#endif /* _APP_H_ */

/** @} */
