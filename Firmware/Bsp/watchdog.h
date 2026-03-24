/**
 * @file       watchdog.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Watchdog module
 *
 * @addtogroup grWatchdog
 * @{
 */

#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Watchdog initialization
 */
extern void watchdog_init();

/**
 * @brief Reset the watchdog
 */
extern void watchdog_hit();

/* ---------------------------------------------------------------------------*/

#endif  /* _WATCHDOG_H_ */

/** @} */
