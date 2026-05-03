/**
 * @file       test_stdio.c
 * @author     OK1CTR
 * @date       May 2026
 * @brief      Test of standard I/Os
 *
 * @addtogroup grApplication
 * @{
 */

#if defined(APP_MODE) && APP_MODE == 100

/* Includes ------------------------------------------------------------------*/

#include <main.h>
#include <app.h>

/* Private defines -----------------------------------------------------------*/

/*! Quick rate task period in SysTick cycles */
#define RUN_PERIOD_QUICK               tick_ms(125)
/*! Medium rate task period in SysTick cycles */
#define RUN_PERIOD_MEDIUM              tick_ms(1000)

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Application related initialization
 */
void app_init()
{
}


/**
 * @brief Application main body
 */
extern void app_run()
{
    ticks_t tmr_run_medium, tmr_run_quick;
    int cnt = 1;

    // before start
    printf("[+] Standard I/O test start\n");

    // periodic tasks
    tick_timer_set(&tmr_run_medium, RUN_PERIOD_MEDIUM);
    tick_timer_set(&tmr_run_quick, RUN_PERIOD_QUICK);

    // infinite loop
    while (1)
    {
        serial_job();

        if (tick_timer_expired(&tmr_run_quick))
        {
            tick_timer_set(&tmr_run_quick, RUN_PERIOD_QUICK);
            watchdog_hit();
        }

        if (tick_timer_expired(&tmr_run_medium))
        {
            tick_timer_set(&tmr_run_medium, RUN_PERIOD_MEDIUM);
            printf("[*] Medium timer expired %d\n", cnt);
            cnt++;
            i2c_led_bar(cnt);
        }
    }
}

/* ---------------------------------------------------------------------------*/

#endif

/** @} */
