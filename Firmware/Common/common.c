/**
 * @file       common.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Common definitions and functions
 *
 * @addtogroup grCommon
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <common.h>
#include <main.h>

/* Functions -----------------------------------------------------------------*/

/* Make short delay based on loop cycles */
void delay_loop(uint32_t us)
{
    const uint32_t cycles = SystemCoreClock * us / 5000000;

    for (uint32_t i = 0; i < cycles; i++)
    {
        __NOP();
    }
}


/* Standard error handler */
void error_handler(void)
{
    __disable_irq();
    while (1)
    {
        __NOP();
    }
}


/* Extended error handler*/
void error_handler_ex(char *file, int line)
{
    __disable_irq();
    while (1)
    {
        __NOP();
    }
}


#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    __NOP();
}
#endif  /* USE_FULL_ASSERT */

/* ---------------------------------------------------------------------------*/

/** @} */
