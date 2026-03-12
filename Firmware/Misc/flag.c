/**
 * @file       flags.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Application flags storage module
 *
 * @addtogroup grFlags
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <flag.h>
#include <stm32_assert.h>
#include <critical.h>

/* Private typedefs ----------------------------------------------------------*/

/* Flag need handle storage typedef */
typedef struct
{
    uint32_t handle;   ///< Each bit represents one flag for one task that need to be handled.
} Flag_Private_t;

/* Private variables ---------------------------------------------------------*/

/* Flag need handle storage variable */
static Flag_Private_t flag;

/* Functions -----------------------------------------------------------------*/

/* Need handle flags storage initialization */
Status_t flag_init(void)
{
    Status_t ret = STATUS_OK;
    flag.handle = 0;
    return ret;
}


/* Set need handle flag */
void flag_set_need_handle(uint32_t flags)
{
    assert_param(IS_FLAG(flags));
    critical_enter();
    flag.handle |= flags;
    critical_exit();
}


/* Get need handle flag */
bool flag_get_need_handle(uint32_t flags)
{
    return (flags & flag.handle);
}


/* Clear need handle flag */
void flag_clear_need_handle(uint32_t flags)
{
    assert_param(IS_FLAG(flags));
    critical_enter();
    flag.handle &= ~flags;
    critical_exit();
}

/* ---------------------------------------------------------------------------*/

/** @} */
