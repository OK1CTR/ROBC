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

/* Standard error handler */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        __NOP();
    }
}


/* Extended error handler*/
void Error_Handler_ex(char *file, int line)
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
