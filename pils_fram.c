/*!
 * \addtogroup Fram FRAM
 * \brief PilsenCUBE COM-OBC ferroelectric RAM storage low level driver
 * @{
 */

/*!
 * \file    pils_fram.c
 * \brief   PilsenCUBE COM-OBC ferroelectric RAM storage low level driver source
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#include <stdint.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "pils_config.h"
#include "pils_fram.h"


/*! @name Static macros for setting of the FRAM chip select wires.
 *  @{
 */
//! Set the MCSA bank chip select signal high
#define fram_mcsa_H() GPIOA->BSRR = (uint32_t)(0x00000001L << 15)
//! Set the MCSA bank chip select signal low
#define fram_mcsa_L() GPIOA->BRR = (uint32_t)(0x00000001L << 15)
//! Set the MCSB bank chip select signal high
#define fram_mcsb_H() GPIOB->BSRR = (uint32_t)(0x00000001L << 8)
//! Set the MCSB bank chip select signal low
#define fram_mcsb_L() GPIOB->BRR = (uint32_t)(0x00000001L << 8)
/*! @} */


//! Required FRAM chip signature
static uint8_t fram_signature[9] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xC2, 0x26, 0x08 };


/* Initialize the SPI1 interface for FRAM memory array, test and list memory chips. */
uint8_t fram_init(uint8_t chips)
{
	uint8_t i, n, p = 0, a = 9;

	// (Re)Init GPIO lines dedicated to SPI1 peripherial
	// PIN SPI - SCK, MISIO, MOSI = PB3, PB4, PB5
	// Attention to JTAG collision, JTAG must be disabled before GPIO init !!!
	GPIOB->CRL &= CONFMASK(3); GPIOB->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_AFIO_PP, 3);
	GPIOB->CRL &= CONFMASK(4); GPIOB->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_I_PULL, 4);  // pullup is good for missing FRAM chips
	GPIOB->CRL &= CONFMASK(5); GPIOB->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_AFIO_PP, 5);
	// Remap the SPI1 interface
	AFIO->MAPR |= AFIO_MAPR_SPI1_REMAP;
#ifdef ROBC_VER_3
	// PIN FRAM - MCSA, MCSB = PA15, PB8
	GPIOA->CRH &= CONFMASK(7); GPIOA->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 7);
	GPIOB->CRH &= CONFMASK(0); GPIOB->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 0);
	fram_mcsa_H(); fram_mcsb_H();
#endif
	
	// Reset SPI1 peripherial
	RCC->APB2RSTR = RCC_APB2RSTR_SPI1RST;
	for (i = 0; i < 255; i++) __nop();
	RCC->APB2RSTR = 0;
	
	// Init SPI1 used by FRAM storage
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SPE | ((FRAM_SPI_BAUD_RATE & 0x7) << 3);
	SPI1->CR2 = SPI_CR2_SSOE;

	// Test connection to FRAM chip and chip array population
	for (i = 0, n = 1; i < 3; i++, n <<= 1) {
		if (!(chips & n)) continue;
		fram_bank_cs(i);
		spi1_poll(FRAM_CMD_RDID);
		a -= (spi1_poll(0) - fram_signature[0]) ? 0 : 1;
		a -= (spi1_poll(0) - fram_signature[1]) ? 0 : 1;
		a -= (spi1_poll(0) - fram_signature[2]) ? 0 : 1;
		a -= (spi1_poll(0) - fram_signature[3]) ? 0 : 1;
		a -= (spi1_poll(0) - fram_signature[4]) ? 0 : 1;
		a -= (spi1_poll(0) - fram_signature[5]) ? 0 : 1;
		a -= (spi1_poll(0) - fram_signature[6]) ? 0 : 1;
		a -= (spi1_poll(0) - fram_signature[7]) ? 0 : 1;
		a -= (spi1_poll(0) - fram_signature[8]) ? 0 : 1;
		fram_bank_cs(i - 1);
		if (!a) p |= n;
	}
	
	return(p);
}


/* Switch the SPI1 clock off. */
void fram_suspend(void)
{
	RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;
	return;
}


/* Select the requested FRAM memory bank (chip) and produces needed CS signal waveform */
uint8_t fram_bank_cs(uint8_t bank)
{
	static uint8_t bank_last = 0x03;
	uint8_t i;
	
	bank &= 0x03;
	if (bank != bank_last) {
		bank_last = bank;
		if (bank & 0x01) fram_mcsa_H(); else fram_mcsa_L();
		if (bank & 0x02) fram_mcsb_H(); else fram_mcsb_L();
	} else {
		bank++;
		if (bank & 0x01) fram_mcsa_H(); else fram_mcsa_L();
		if (bank & 0x02) fram_mcsb_H(); else fram_mcsb_L();
		for (i = 0; i < 255; i++) __nop();
		bank--;
		if (bank & 0x01) fram_mcsa_H(); else fram_mcsa_L();
		if (bank & 0x02) fram_mcsb_H(); else fram_mcsb_L();
	}

	return(bank_last);
}


/* Set the block write protection in selected FRAM chip */
uint8_t fram_block_prot(uint8_t bank, uint8_t protection)
{
	uint8_t a;

	// Write enable
	fram_bank_cs(bank);
	spi1_poll(FRAM_CMD_WREN);
	fram_bank_cs(bank + 1);
	// Write the status register
	fram_bank_cs(bank);
	spi1_poll(FRAM_CMD_WRSR);
	spi1_poll(((protection & 0x3) << 2) | 0x40);
	fram_bank_cs(bank + 1);
	// Read the status register back and check the protection
	fram_bank_cs(bank);
	spi1_poll(FRAM_CMD_RDSR);
	a = spi1_poll(0);
	fram_bank_cs(bank + 1);
	return((((a >> 2) & 0x3) == protection) ? 0 : 1);
}


/* Write the buffer contents to the FRAM memory */
void fram_write(uint8_t bank, uint32_t address, uint32_t count, uint8_t *data)
{
	uint8_t *p = data;

	// Write enable
	fram_bank_cs(bank);
	spi1_poll(FRAM_CMD_WREN);
	fram_bank_cs(bank + 1);
	// Write address
	fram_bank_cs(bank);
	spi1_poll(FRAM_CMD_WRITE);
	address &= 0x7FFFF;
	spi1_poll(address >> 16);
	spi1_poll((address >> 8) & 0xFF);
	spi1_poll(address & 0xFF);
	// Write data
	while (count) {
		spi1_poll(*p);
		count--; p++;
	}
	fram_bank_cs(bank + 1);
	return;
}


/* Read the FRAM memory data and store it into the buffer */
void fram_read(uint8_t bank, uint32_t address, uint32_t count, uint8_t *data)
{
	uint8_t *p = data;

	// Write address
	fram_bank_cs(bank);
	spi1_poll(FRAM_CMD_READ);
	address &= 0x7FFFF;
	spi1_poll(address >> 16);
	spi1_poll((address >> 8) & 0xFF);
	spi1_poll(address & 0xFF);
	// Write data
	while (count) {
		*p = spi1_poll(0);
		count--; p++;
	}
	fram_bank_cs(bank + 1);
	return;
}


/* Simple SPI1 byte exchange using status register polling */
uint8_t spi1_poll(uint8_t data)
{
	SPI1->DR = data;
	while (!(SPI1->SR & SPI_SR_TXE))
		;
	while (!(SPI1->SR & SPI_SR_RXNE))
		;
	return(SPI1->DR);
}

/*! @} */
