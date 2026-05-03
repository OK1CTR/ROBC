/**
 * @file       main.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      RadioOBC main module
 *
 * @addtogroup grMain
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <main.h>
#include <app.h>

/* Functions -----------------------------------------------------------------*/

/* The application entry point */
int main(void)
{
    // modules initialization
    msp_init();
    gpio_init();
    watchdog_init();
    tick_init();
    rtc_init();
    serial_init();
    i2c_init();
    flag_init();
    spi_emu_init();
    ax_init();
    radio_init();

    // application related settings
    app_init();
    // application main body
    app_run();
}

/* ---------------------------------------------------------------------------*/

/** @} */
