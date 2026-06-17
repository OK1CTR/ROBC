/**
 * @file       test_hdlc_rx.c
 * @author     OK1CTR
 * @date       May 2026
 * @brief      Test of GMSK HDLC receiver station
 *
 * @addtogroup grApplication
 * @{
 */

#if defined(APP_MODE) && APP_MODE == 105

/* Includes ------------------------------------------------------------------*/

#include <main.h>
#include <app.h>
#include <stdlib.h>

/* Private defines -----------------------------------------------------------*/

/*! Quick rate task period in SysTick cycles */
#define RUN_PERIOD_QUICK               tick_ms(125)
/*! Medium rate task period in SysTick cycles */
#define RUN_PERIOD_MEDIUM              tick_ms(2000)

//! Test frame payload length in bytes
#define TEST_PAYLOAD_LEN               64

/* Private typedefs ----------------------------------------------------------*/

/*! Test data frame */
/*
typedef union
{
    uint8_t data[TEST_PAYLOAD_LEN];
    struct
    {
        uint32_t nbr;
        uint8_t fill[TEST_PAYLOAD_LEN - 4];
    };
} test_frame_t;
*/
uint8_t rx_buf[256];

/*! Radio state machine */
typedef enum
{
    radio_idle = 03,                     ///< waiting to trigger
    radio_rx,                            ///< reception
} radio_state_e;

/* Private variables ---------------------------------------------------------*/

/*! Test frame */
//static test_frame_t frame;

/*! Radio state machine */
static radio_state_e state = radio_idle;

/* Private function prototypes -----------------------------------------------*/
/* Functions -----------------------------------------------------------------*/

/**
 * @brief Application related initialization
 */
void app_init()
{
    radio_init();
    ax_mode_g3ruh(ax_g3ruh_rate_9600, ax_crc_mode_crc32, ax_enc_mode_scramnler);
    //ax25_init();
    ax_crc_init();
    ax_fifo_init();
    i2c_led_bar(0);
}


/**
 * @brief Application main body
 */
extern void app_run()
{
    ticks_t tmr_run_medium, tmr_run_quick;
    uint32_t cnt = 0, len;

    // before start
    printf("[+] HDLC GMSK receiver station test\n");

    // periodic tasks
    tick_timer_set(&tmr_run_medium, RUN_PERIOD_MEDIUM);
    tick_timer_set(&tmr_run_quick, RUN_PERIOD_QUICK);

    flag_clear_need_handle(FLAG_RADIO_DONE);
    flag_clear_need_handle(FLAG_RADIO_STATE);
    flag_clear_need_handle(FLAG_RADIO_DATA_GET);
    ax_irq_enable(false, true, false);

    // infinite loop
    while (1)
    {
        serial_job();

        if (tick_timer_expired(&tmr_run_quick))
        {
            tick_timer_set(&tmr_run_quick, RUN_PERIOD_QUICK);
            watchdog_hit();

            if (state == radio_idle)
            {
                printf("[*] Receive ...\n");
                ax_pwrmode(ax_pwrmode_rx);
                state = radio_rx;
            }
        }

        if (tick_timer_expired(&tmr_run_medium))
        {
            tick_timer_set(&tmr_run_medium, RUN_PERIOD_MEDIUM);
            // currently nothing to do
        }

        if (flag_get_need_handle(FLAG_RADIO_STATE))
        {
            flag_clear_need_handle(FLAG_RADIO_STATE);
#if 0
            printf("[*] STATE\n");
#elif 1
            printf("[*] radio state = %01X\n", (int)ax_get_radio_state());
#else
            if (ax_get_radio_state() == ax_radiostate_rx_ant_set)
            {
                printf("[*] radio state = %d\n", ax_get_rssi_bg());
            }
#endif
        }

        if (flag_get_need_handle(FLAG_RADIO_DONE))
        {
            flag_clear_need_handle(FLAG_RADIO_DONE);
            printf("[*] DONE\n");
        }


        if (flag_get_need_handle(FLAG_RADIO_DATA_GET))
        {
            flag_clear_need_handle(FLAG_RADIO_DATA_GET);
            if (state != radio_rx)
            {
                printf("[-] Prohibited state if IRQ active!\n");
            }
            else
            {
                // *** ax_pwrmode(ax_pwrmode_rx_synt);
                len = ax_fifo_read(rx_buf, 256);
                ax_fifo_cmd(ax_fifo_cmd_clr_fifo);

                //hdlc_read_msg(frame.data, TEST_PAYLOAD_LEN, 3);  // TODO position parameter
                printf("[*] Message received: %lu, %lu bytes long\n", cnt, len);
                i2c_led_bar(cnt);
                cnt++;
                //state = radio_idle;
            }
        }
    }
}

/* Private functions ---------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

#endif

/** @} */
