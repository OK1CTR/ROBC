/*!
 * \addtogroup Meas Measurement
 * \brief PilsenCUBE COM-OBC measurement module for internal and external ADC
 * @{
 */

/*!
 * \file    pils_adc.c
 * \brief   PilsenCUBE COM-OBC measurement module for internal and external ADC, source
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#include <stdint.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "i2c.h"
#include "pils_adc.h"


#if !defined(ROBC_VER_2) && !defined(ROBC_VER_3)
	#error Define the ROBC board version!
#endif


/* Reset and initialize the I2C ADC */
uint8_t i2c_adc_init(void)
{
	uint8_t ret;

	i2c_bf1[0] = 0x00; i2c_bf1[1] = 0x00;
	i2c1_write_buf(0x2F, 0, 2);
	i2c1_wait(); sleep_us(1000);
	ret = i2c1_error; i2c1_error = 0;
	return(ret);
}


/* Perform a measurement on the I2C ADC */
uint8_t i2c_adc_meas(uint16_t *result, uint8_t chan)
{
	uint8_t ret = 0;
	uint16_t res;
	
	// start the measurement
	if (chan < 8) {
		// channel 0 - 7 (voltage)
		i2c_bf1[0] = 1 << (7 - chan); i2c_bf1[1] = 0x00;
	} else {
		// channel 8 (temperature)
		i2c_bf1[0] = 0; i2c_bf1[1] = 0x80;
	}
	i2c1_write_buf(0x2F, 0, 2);
	i2c1_wait(); sleep_us(1000);
	ret = i2c1_error; i2c1_error = 0;
	if (ret) return(i2c1_error);

	// read the data
	i2c1_read_buf(0x2F, (chan < 8) ? 1 : 2, 2);
	i2c1_wait(); sleep_us(1000);
	res = i2c_bf1[0]; res <<= 8; res |= i2c_bf1[1];
	*result = res & 0x0FFF; // 4 MSB masked - channel number
	ret = i2c1_error; i2c1_error = 0;
	return(ret);
}


/* Reset and calibrate the internal STM32 ADC */
void stm_adc_init(void)
{
	uint8_t n;

	// analog pin configuration
#ifdef ROBC_VER_2
	// PIN Analog inputs - ADC0, ADC4, ADC5, ADC6, ADC8, ADC9 = PA0, PA4, PA5, PA6, PB0, PB1
	GPIOA->CRL &= CONFMASK(0); GPIOA->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 0);
	GPIOA->CRL &= CONFMASK(4); GPIOA->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 4);
	GPIOA->CRL &= CONFMASK(5); GPIOA->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 5);
	GPIOA->CRL &= CONFMASK(6); GPIOA->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 6);
	GPIOB->CRL &= CONFMASK(0); GPIOB->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 0);
	GPIOB->CRL &= CONFMASK(1); GPIOB->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 1);
#endif

#ifdef ROBC_VER_3
	// PIN Analog inputs - ADC5, ADC6, ADC7, ADC8, ADC9 = PA5, PA6, PA7, PB0, PB1
	GPIOA->CRL &= CONFMASK(5); GPIOA->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 5);
	GPIOA->CRL &= CONFMASK(6); GPIOA->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 6);
	GPIOA->CRL &= CONFMASK(7); GPIOA->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 7);
	GPIOB->CRL &= CONFMASK(0); GPIOB->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 0);
	GPIOB->CRL &= CONFMASK(1); GPIOB->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_ANALOG, 1);
#endif
	
	// reset ADC1 peripherial
	RCC->APB2RSTR = RCC_APB2RSTR_ADC1RST;
	for (n = 0; n < 255; n++) __nop();
	RCC->APB2RSTR = 0;
	// clock configuration
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV4;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	// calibration
	ADC1->CR2 |= ADC_CR2_ADON;
	sleep_us(100);
	ADC1->CR2 |= ADC_CR2_CAL;
	while ((ADC1->CR2 & ADC_CR2_CAL) == ADC_CR2_CAL)
		;
	// configuration
	ADC1->CR1 = 0;
	ADC1->SMPR1 = 0;  // if works bad, set longer sample time
	ADC1->SMPR2 = 0;  // if works bad, set longer sample time
	return;
}


/* Perform a measurement on the internal STM32 ADC */
void stm_adc_meas(uint16_t *result, uint8_t chan)
{
#ifdef ROBC_VER_2
	switch (chan) {
		case 0:
			ADC1->SQR3 = 0; break;
		case 1:
		case 2:
		case 3:
			ADC1->SQR3 = chan + 3; break;
		case 4:
		case 5:
			ADC1->SQR3 = chan + 4; break;
		default:
			// if works bad, set a special sample time (from manual)
			ADC1->CR2 |= ADC_CR2_TSVREFE;
			ADC1->SQR3 = (chan - 6 + 0x10) & 0x1F;
	}
	ADC1->SQR1 = 1 << 20;
#endif
	
#ifdef ROBC_VER_3
	if (chan < 5) {
		ADC1->SQR3 = chan + 5;
	} else {
		ADC1->CR2 |= ADC_CR2_TSVREFE;
		ADC1->SQR3 = (chan - 5 + 0x10) & 0x1F;
		// if works bad, set a special sample time (from manual)
	}
	ADC1->SQR1 = 1 << 20;
#endif
	// conversion
	ADC1->CR2 |= ADC_CR2_ADON;  // just touch this bit to start conversion
	while (!(ADC1->SR & ADC_SR_EOC))
		;
	*result = ADC1->DR;
	// temperature sensor and Vref off
	if (chan >= 5) {
		ADC1->CR2 &= ~ADC_CR2_TSVREFE;
	}
	return;
}


/*! @} */
