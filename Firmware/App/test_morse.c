/**
 * @file       test_morse.c
 * @author     OK1CTR
 * @date       May 2026
 * @brief      Test of Morse transmission
 *
 * @addtogroup grApplication
 * @{
 */

#if defined(APP_MODE) && APP_MODE == 101

/* Includes ------------------------------------------------------------------*/

#include <main.h>
#include <app.h>

/* Private defines -----------------------------------------------------------*/

/*! Morse transmission rate */
#define MORSE_RATE_WPM                40

/*! Quick rate task period in SysTick cycles */
#define RUN_PERIOD_QUICK               tick_ms(125)
/*! Medium rate task period in SysTick cycles */
#define RUN_PERIOD_MEDIUM              tick_ms(10000)

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Application related initialization
 */
void app_init()
{
    radio_init();
    ax_pwrmode(ax_pwrmode_tx_synt);
    morse_init(MORSE_RATE_WPM);
    ax_pwrmode(ax_pwrmode_tx);
}


/**
 * @brief Application main body
 */
extern void app_run()
{
    ticks_t tmr_run_medium, tmr_run_quick;
    int cnt = 1;
    bool fifo_empty = false;
    bool tx_progress = false;

    // before start
    printf("[+] Morse test start\n");

    // periodic tasks
    tick_timer_set(&tmr_run_medium, RUN_PERIOD_MEDIUM);
    tick_timer_set(&tmr_run_quick, RUN_PERIOD_QUICK);

    // infinite loop
    while (1)
    {
        serial_job();
        morse_job();

        if (tick_timer_expired(&tmr_run_quick))
        {
            tick_timer_set(&tmr_run_quick, RUN_PERIOD_QUICK);
            watchdog_hit();
        }

        if (tick_timer_expired(&tmr_run_medium))
        {
            tick_timer_set(&tmr_run_medium, RUN_PERIOD_MEDIUM);
            morse_send("ahoj");
            printf("[*] Message set %d\n", cnt);
            cnt++;
            i2c_led_bar(cnt);
            fifo_empty = false;
            tx_progress = false;
        }

        if (tx_progress == false && !morse_is_transmit())
        {
            tx_progress = true;
            printf("[*] Transmission finished\n");
        }

        if (fifo_empty == false && morse_is_fifo_empty())
        {
            fifo_empty = true;
            printf("[*] FIFO empty\n");
        }
    }
}

/* ---------------------------------------------------------------------------*/

#endif

/** @} */
