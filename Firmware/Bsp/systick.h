/**
 * @file       systick.h
 * @author     OK1CTR
 * @date       Mar 2026
 * @brief      MCU core SzsTick based timing
 *
 * @addtogroup grSystick
 * @{
 */

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/* Defines -------------------------------------------------------------------*/

//! Number of SusTick ticks per second
#define TICK_FREQUENCY              1000

/* Macros --------------------------------------------------------------------*/

//! Return number of ticks corresponding to milliseconds
#define tick_ms(a)                  ((TICK_FREQUENCY) * (a) / 1000)

//! Return number of ticks corresponding to seconds
#define tick_s(a)                   ((TICK_FREQUENCY) * (a))

//! Return number of ticks corresponding to minutes
#define tick_min(a)                 ((TICK_FREQUENCY) * 60 * (a))

//! Return number of ticks corresponding to hours
#define tick_hour(a)                ((TICK_FREQUENCY) * 3600 * (a))

/* Typedefs ------------------------------------------------------------------*/

/* 64bit integer to maintain time as a number of clock ticks */
typedef uint64_t ticks_t;

/* Functions -----------------------------------------------------------------*/

/**
 * @brief SysTick initialization
 */
extern void tick_init();

/**
 * @brief Return actual state of the SysTick tick counter
 */
extern ticks_t ticks_now();

/**
 * @brief Set 64bit unsigned tick timer to given number of ticks
 * @param tmr Pointer to systick software timer
 * @param ticks Number of ticks to timer expire
 */
extern void tick_timer_set(ticks_t *tmr, ticks_t ticks);

/**
 * @brief Check 64bit unsigned tick timer to expire
 * @param tmr Pointer to systick software timer
 * @return True if timer expired
 */
extern bool tick_timer_expired(ticks_t *tmr);

/**
 * @brief Delay for given number of ticks
 * @param ticks Given number of ticks
 */
extern void tick_delay(uint32_t ticks);

/* ---------------------------------------------------------------------------*/

#endif  /* _SYSTICK_H_ */

/** @} */
