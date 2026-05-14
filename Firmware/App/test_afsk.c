/**
 * @file       test_afsk.c
 * @author     OK1CTR
 * @date       May 2026
 * @brief      Test of AFSK transmission
 *
 * @addtogroup grApplication
 * @{
 */

#if defined(APP_MODE) && APP_MODE == 102

/* Includes ------------------------------------------------------------------*/

#include <main.h>
#include <app.h>

/* Private defines -----------------------------------------------------------*/

/*! Quick rate task period in SysTick cycles */
#define RUN_PERIOD_QUICK               tick_ms(125)
/*! Medium rate task period in SysTick cycles */
#define RUN_PERIOD_MEDIUM              tick_ms(1000)

//! AX.25 test message buffer length
#define BUF_TX_LEN                     65
//! AX.25 test message constant (50 chars + 14 spaces)
#define FRAME_TEMPLATE "The PilsenCUBE satellite COM/OBC test @ @ @ @ \x7F\xFE\x7F\xFE              "

/* Private variables ---------------------------------------------------------*/

//! Test mesage buffer
static uint8_t buf_tx[BUF_TX_LEN];

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Application related initialization
 */
void app_init()
{
    radio_init();
    ax_pwrmode(ax_pwrmode_tx_synt);
    ax_mode_afsk(ax_crc_mode_off);
    ax_pwrmode(ax_pwrmode_tx);
    ax25_init();
    ax_fifo_init();

    // test message initialization
    memset(buf_tx, '.', BUF_TX_LEN);
    strcpy((char *)buf_tx, FRAME_TEMPLATE);
}


/**
 * @brief Application main body
 */
extern void app_run()
{
    ticks_t tmr_run_medium, tmr_run_quick;
    int cnt = 1;

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
            // ax25_send_flag(10);
            // ax25_send_msg((unsigned char*) "HOVNO PRDEL SRACKA TO JE NASE ZNACKA", 0);
            // ax25_send_msg((unsigned char*) "PPPPPPPPPP PRDELLL PRDELLL PRDELL !!!!!", 0);
            // ax25_send_msg((unsigned char*) "v", 0);
            sprintf((char *)(buf_tx + 51), "%08X", cnt);
            printf("[*] Message sent: %s\n", buf_tx);
            ax25_send_msg(buf_tx, 0);
            cnt++;
            i2c_led_bar(cnt);
        }
    }
}

/* ---------------------------------------------------------------------------*/

#endif

/** @} */
