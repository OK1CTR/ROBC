/*!
 * \addtogroup Meas Measurement
 * \brief PilsenCUBE COM-OBC measurement module for internal and external ADC
 * @{
 */

/*!
 * \file    pils_adc.h
 * \brief   PilsenCUBE COM-OBC measurement module for internal and external ADC, header
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#ifndef _PILS_ADC_H_
#define _PILS_ADC_H_


/*! \brief Reset and initialize the I2C ADC.
 *  \return The I2C error value. If cusscess, return 0. The I2C error flag is deleted.
 */
extern uint8_t i2c_adc_init(void);

/*! \brief Perform a measurement on the I2C ADC.
 *  \param *result Pointer where to store the result.
 *  \param chan Channel number to read, voltage channel 0 - 7 or 8 for temperature.
 *  \return The I2C error value. If cusscess, return 0. The I2C error flag is deleted.
 */
extern uint8_t i2c_adc_meas(uint16_t *result, uint8_t chan);

/*! \brief Reset and calibrate the internal STM32 ADC
 */
extern void stm_adc_init(void);

/*! \brief Perform a measurement on the internal STM32 ADC
 *  \param result Pointer where to store the result.
 *  \param chan Channel number to read, 0-6 renubered from 5-9, temperature, Vref. For version 2 HW the remapping is more complex :-)
 */
void stm_adc_meas(uint16_t *result, uint8_t chan);

#endif

/*! @} */
