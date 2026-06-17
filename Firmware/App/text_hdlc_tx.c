/**
 * @file       test_hdlc_tx.c
 * @author     OK1CTR
 * @date       May 2026
 * @brief      Test of GMSK HDLC transmitter station
 *
 * @addtogroup grApplication
 * @{
 */

#if defined(APP_MODE) && APP_MODE == 104

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
typedef union
{
    uint8_t data[TEST_PAYLOAD_LEN];
    struct
    {
        uint32_t nbr;
        uint8_t fill[TEST_PAYLOAD_LEN - 4];
    };
} test_frame_t;

/*! Radio state machine */
typedef enum
{
    radio_idle = 03,                     ///< waiting to trigger
    radio_tx,                            ///< transmission
} radio_state_e;

/* Private variables ---------------------------------------------------------*/

/*! Test frame */
static test_frame_t frame;

/*! Radio state machine */
static radio_state_e state = radio_idle;

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Generate pseudo random data to fill the frame
 * @param dest Pointer to destination buffer
 * @param length Buffer length
 */
static void data_generate(uint8_t *dest, uint32_t length);

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Application related initialization
 */
void app_init()
{
    radio_init();
    ax_pwrmode(ax_pwrmode_tx_synt);
    ax_mode_g3ruh(ax_g3ruh_rate_9600, ax_crc_mode_crc32, ax_enc_mode_scramnler);
    ax25_init();
    ax_crc_init();
    ax_fifo_init();
    srand(1324);
}


/**
 * @brief Application main body
 */
extern void app_run()
{
    ticks_t tmr_run_medium, tmr_run_quick;
    uint32_t cnt = 0;

    // before start
    printf("[+] HDLC GMSK transmitter station test\n");

    // periodic tasks
    tick_timer_set(&tmr_run_medium, RUN_PERIOD_MEDIUM);
    tick_timer_set(&tmr_run_quick, RUN_PERIOD_QUICK);

    flag_clear_need_handle(FLAG_RADIO_DONE);
    ax_set_irq_done_enable(true);


    ax_pwrmode(ax_pwrmode_tx);  // ***

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

            if (state != radio_idle)
            {
                printf("[-] Prohibited state if starting TX!\n");
            }
            else
            {
                printf("[*] Message sent: %lu\n", cnt);
                frame.nbr = cnt;
                /* *** */

#define AX25_M00_LEN 65
#define MSG_SER_N 16
#define HDLC_PKT_CONST "%08X-%02X-%02X...PilsenCUBE-SATELITE-UWB-PILSEN-0123456789ABCDEF"

                static uint8_t msg00[AX25_M00_LEN];
                uint8_t msg_ser_n = 0, a;
                for (msg_ser_n = 0; msg_ser_n < MSG_SER_N; msg_ser_n++)
                {
                    sprintf((char *) msg00, HDLC_PKT_CONST, (unsigned int)cnt, msg_ser_n, MSG_SER_N);
                    if (msg_ser_n == 0) a = SER_POS_1; else a = 0;
                    if (msg_ser_n == MSG_SER_N  - 1) a |= SER_POS_N;
                    hdlc_send_msg(msg00, 64, a);
                }

                /* *** */
                // *** data_generate(frame.fill, TEST_PAYLOAD_LEN - 4);
                // ***hdlc_send_msg(frame.data, TEST_PAYLOAD_LEN, 3);  // TODO position parameter
                // *** ax_pwrmode(ax_pwrmode_tx);
                i2c_led_bar(cnt);
                cnt++;
                // *** state = radio_tx;
            }
        }

        if (flag_get_need_handle(FLAG_RADIO_DONE))
        {
            flag_clear_need_handle(FLAG_RADIO_DONE);

            if (state != radio_tx)
            {
                printf("[-] Prohibited state if IRQ active!\n");
            }
            else
            {
                // *** ax_pwrmode(ax_pwrmode_tx_synt);
                printf("[*] Transmission done\n");
                state = radio_idle;
            }
        }
    }
}

/* Private functions ---------------------------------------------------------*/

/* Generate pseudo random data to fill the frame */
static void data_generate(uint8_t *dest, uint32_t length)
{
   uint32_t i;

   for (i = 0; i < length; i++)
   {
       dest[i] = rand() % 256;
   }
}

/* ---------------------------------------------------------------------------*/

#endif

/** @} */
