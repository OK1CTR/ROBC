/**
 * @file       critical.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Disable interrupts recursively for critical code sections
 *
 * @addtogroup grCritical
 * @{
 */

#ifndef _CRITICAL_H_
#define _CRITICAL_H_

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Begin of critical section
 */
extern void critical_enter();

/**
 * @brief End of critical section
 */
extern void critical_exit();

/* ---------------------------------------------------------------------------*/

#endif  /* _CRITICAL_H_ */

/** @} */
