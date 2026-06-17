/**
 * @file       flags.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Application flags storage module
 *
 * @addtogroup grFlags
 * @{
 */

#ifndef _APP_FLAGS_H_
#define _APP_FLAGS_H_

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <common.h>

/* Definitions----------------------------------------------------------------*/

/** @defgroup grFlagDefs
 *  @ingroup grFlag
 *  @brief Flag bit-wise define macros
 *  @{
 */

#define FLAG_SECOND             (1 << 0)    ///< Global timing - second flag
#define FLAG_MINUTE             (1 << 1)    ///< Global timing - minute flag
#define FLAG_ALARM_1            (1 << 2)    ///< Global timing - alarm clock 1
#define FLAG_ALARM_2            (1 << 3)    ///< Global timing - alarm clock 2
#define FLAG_RADIO_STATE        (1 << 4)    ///< Radio - radio state changed
#define FLAG_RADIO_DONE         (1 << 5)    ///< Radio - process done
#define FLAG_RADIO_DATA_PUT     (1 << 6)    ///< Radio - FIFO ready to put data in
#define FLAG_RADIO_DATA_GET     (1 << 7)    ///< Radio - FIFO ready to get data from

/** @} */

//! Mask of all flags. This is used to verify flag parameters
#define FLAG_ALL (FLAG_SECOND\
                  | FLAG_MINUTE\
                  | FLAG_ALARM_1\
                  | FLAG_ALARM_2\
                  | FLAG_RADIO_STATE\
                  | FLAG_RADIO_DONE\
                  | FLAG_RADIO_DATA_PUT\
                  | FLAG_RADIO_DATA_GET)

#define IS_FLAG(a) (a & FLAG_ALL)

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Need handle flags storage initialization
 * @return STATUS_OK
 */
extern Status_t flag_init(void);

/**
 * @brief Set need handle flag
 * @param flags Bit-wise sum of flags to be set
 */
extern void flag_set_need_handle(uint32_t flags);

/**
 * @brief Get need handle flag
 * @param flags Bit-wise sum of flags to get
 * @return Value of flags in question
 */
extern bool flag_get_need_handle(uint32_t flags);

/**
 * @brief Clear need handle flag
 * @param flags Bit-wise sum of flags to be cleared
 */
extern void flag_clear_need_handle(uint32_t flags);

/* ---------------------------------------------------------------------------*/

#endif /* _APP_FLAGS_H_ */

/** @} */
