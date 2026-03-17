/**
 * @file       iwdg.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Independent watchdog module
 *
 * @addtogroup grIwdg
 * @{
 */

#ifndef _IWDG_H_
#define _IWDG_H_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* Functions -----------------------------------------------------------------*/

/**
 * @brief IWDG initialization
 */
extern void iwdg_init();

/**
 * @brief Reset the IWDG timer
 */
extern void iwdg_hit();

/* ---------------------------------------------------------------------------*/

#endif  /* _IWDG_H_ */

/** @} */
