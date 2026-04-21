/**
 * @file       main.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      RadioOBC main module
 *
 * @addtogroup grCore
 * @{
 */

#ifndef _MAIN_H_
#define _MAIN_H_

/* Includes ------------------------------------------------------------------*/

#include <common.h>

#if defined BSP_HW_VER_2
#include <bsp_hwver2.h>
#elif defined BSP_HW_VER_3
#include <bsp_hwver3.h>
#else
#error "HW version is not specified!"
#endif

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif  /* USE_FULL_ASSERT */

/* ---------------------------------------------------------------------------*/

#endif  /* _MAIN_H_ */

/** @} */
