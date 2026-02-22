/*!
 * \addtogroup AXRadio AX5043
 * \brief AX5043 radio driver with HW/SW SPI support
 * @{
 */
 
/*!
 * \file    ax5043.c
 * \brief   AX5043 radio driver with HW/SW SPI support, source
 * \author  OK1CTR
 * \version 1.0
 * \date    21.08.2018
 */


#include <stdint.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "pils_config.h"
#include "robc_hal.h"
#include "ax5043.h"


#if !defined(ROBC_VER_2) && !defined(ROBC_VER_3)
	#error Define the ROBC board version!
#endif

#ifdef ROBC_VER_2
	//! RSEL line software control - H
	#define ax_sel_H() GPIOB->BSRR = (uint32_t) (0x00000001L << 8)
	//! RSEL line software control - L
	#define ax_sel_L() GPIOB->BRR = (uint32_t) (0x00000001L << 8)
	//! RMOSI line software control - H
	#define ax_mosi_H() GPIOB->BSRR = (uint32_t) (0x00000001L << 11)
	//! RMOSI line software control - L
	#define ax_mosi_L() GPIOB->BRR  = (uint32_t) (0x00000001L << 11)
	//! RSCK line software control - H
	#define ax_sck_H() GPIOB->BSRR  = (uint32_t) (0x00000001L << 12)
	//! RSCK line software control - L
	#define ax_sck_L() GPIOB->BRR  = (uint32_t) (0x00000001L << 12)
	//! Read the SPI line RMISO
	#define ax_miso() (GPIOB->IDR & (0x00000001L << 10))
#endif

#ifdef ROBC_VER_3
	//! RSEL line software control - H
	#define ax_rsel_H() GPIOB->BSRR = (uint32_t) (0x00000001L << 12)
	//! RSEL line software control - L
	#define ax_rsel_L() GPIOB->BRR = (uint32_t) (0x00000001L << 12)
#endif


//! Last status word of the AX5043 radio
uint16_t ax_status = 0;
//! AX5043 startup status
uint32_t ax_startup = 0;
//! State of the AX5043 packet tracnsceiver
volatile uint8_t ax_trx_st = AX_TRX_WAIT;

//! Performance tuning registers - progmem array
static const uint8_t rcon_ptrg_prog[RCON_PTRG_LEN] = { RCON_F00, RCON_F0C, RCON_F0D, RCON_F10, RCON_F11, RCON_F1C, RCON_F21, RCON_F22, RCON_F23, RCON_F26, RCON_F34, RCON_F35, RCON_F44, RCON_F72, RCON_188, RCON_189 };
//! Performance tuning registers - RAM array
uint8_t rcon_ptrg_ram[RCON_PTRG_LEN];
//! FM transmitter deviation setting - progmem array
static const uint8_t rcon_fmtx_prog[RCON_FMTX_LEN] = { FMTX_DEV2, FMTX_DEV1, FMTX_DEV0 };
//! FM transmitter deviation setting - RAM array
uint8_t rcon_fmtx_ram[RCON_FMTX_LEN];
//! AFSK transmitter data rate and tone setting - progmem array
static const uint8_t rcon_aftx_prog[RCON_AFTX_LEN] = { AFTX_RATE2, AFTX_RATE1, AFTX_RATE0, AFTX_SPCE1, AFTX_SPCE0, AFTX_MARK1, AFTX_MARK0 };
//! AFSK transmitter data rate and tone setting - RAM array
uint8_t rcon_aftx_ram[RCON_AFTX_LEN];
//! GMSK transmitter dividers - progmem array
static const uint16_t rcon_gdtx_prog[RCON_GDTX_LEN] = { GDTX_1200, GDTX_2400, GDTX_4800, GDTX_9600, GDTX_19200, GSHAPING };
//! GMSK transmitter divider, custom value - RAM
uint16_t rcon_gdtx_ram;


/*! \brief External interrupt handler - RIRQ at AX5043, PB9 in STM32
 */
void EXTI9_5_IRQHandler(void)
{
	if ((EXTI->PR & (1 << 9)) == (1 << 9)) {
		EXTI->PR |= 1 << 9;  // clear pending flag
		if (ax_trx_st == AX_TRX_WAIT) ax_trx_st = AX_TRX_DATA;  // register the incomming data
			else ax_trx_st = AX_TRX_ERROR;  // or packet overrun error
	}

	return;
}


/* Load default AX5043 parameter sets from FLASH to RAM */
void ax_load_par(void)
{
	// do always, nothing to change
	memcpy(rcon_ptrg_ram, rcon_ptrg_prog, RCON_PTRG_LEN);
	// custom value of the GMSK TX divider
	rcon_gdtx_ram = GDTX_9600;
	// FM transmitter
	memcpy(rcon_fmtx_ram, rcon_fmtx_prog, RCON_FMTX_LEN);
	// AFSK transmitter
	memcpy(rcon_aftx_ram, rcon_aftx_prog, RCON_AFTX_LEN);
	return;
}


/* The AX5043 radio short (16bit) Rd/Wr access */
uint8_t ax_rw_2(uint8_t write, uint8_t adr, uint8_t data)
{
	int8_t a;

#ifdef ROBC_VER_2
	int8_t i;
	uint16_t n, m;

	ax_sel_L();
	sleep_us(SPI_DELAY);

	for (i = 0; i < 16; i++) {
		// MOSI control
		if (i == 0) {
			if (write != 0) ax_mosi_H(); else ax_mosi_L();
		}
		if (i == 1) n = 0x40;
		if (i >= 1 && i < 8) {
			if (adr & n) ax_mosi_H(); else ax_mosi_L();
			n >>= 1;
		}
		if (i == 8) n = 0x80;
		if (i >= 8) {
			if (data & n) ax_mosi_H(); else ax_mosi_L();
			n >>= 1;
		}
		sleep_us(SPI_DELAY);

		// MISO read
		if (i == 1) { m = 0x4000; /*ax_status &= 0x7F00; TEST !!!! */ ax_status &= 0x00FF; }
		if (i >= 1 && i < 8) {
			if (ax_miso()) ax_status |= m;
			m >>= 1;
		}
		if (i == 8) { m = 0x80; a = 0; }
		if (i >= 8) {
			if (ax_miso()) a |= m;
			m >>= 1;
		}

		// SCK control
		ax_sck_H(); sleep_us(SPI_DELAY); ax_sck_L(); sleep_us(SPI_DELAY);
	}

	// End of transmission
	ax_sel_H(); ax_mosi_L();	sleep_us(SPI_DELAY);
#endif

#ifdef ROBC_VER_3
	ax_rsel_L();
	// Octet 1
	SPI2->DR = ((write) ? 0x80 : 0) | (adr & 0x7F);
	while (!(SPI2->SR & SPI_SR_TXE))
		;
	while (!(SPI2->SR & SPI_SR_RXNE))
		;
	//ax_status &= 0x7F00; TEST !!!!
	ax_status &= 0x00FF;
	ax_status |= (((uint16_t) SPI2->DR) << 8) & 0x7F00;
	
	// Octet 2
	SPI2->DR = (write) ? data : 0;
	while (!(SPI2->SR & SPI_SR_TXE))
		;
	while (!(SPI2->SR & SPI_SR_RXNE))
		;
	a = SPI2->DR;
	ax_rsel_H();
#endif

	return(a);
}


/* The AX5043 radio medium (24bit) Rd/Wr access */
uint8_t ax_rw_3(uint8_t write, uint16_t adr, uint8_t data)
{
	int8_t a;

#ifdef ROBC_VER_2
	int8_t i;
	uint16_t n, m;

	ax_sel_L();
	sleep_us(SPI_DELAY);

	for (i = 0; i < 24; i++) {
		// MOSI control
		if (i == 0) {
			if (write != 0) ax_mosi_H(); else ax_mosi_L();
		}
		if (i > 0 && i < 4) ax_mosi_H();
		if (i == 4) n = 0x800;
		if (i >= 4 && i < 16) {
			if (adr & n) ax_mosi_H(); else ax_mosi_L();
			n >>= 1;
		}
		if (i == 16) n = 0x80;
		if (i >= 16) {
			if (data & n) ax_mosi_H(); else ax_mosi_L();
			n >>= 1;
		}
		sleep_us(SPI_DELAY);

		// MISO read
		if (i == 1) { m = 0x4000; ax_status = 0; }
		if (i >= 1 && i < 16) {
			if (ax_miso()) ax_status |= m;
			m >>= 1;
		}
		if (i == 16) { m = 0x80; a = 0; }
		if (i >= 16) {
			if (ax_miso()) a |= m;
			m >>= 1;
		}

		// SCK control
		ax_sck_H(); sleep_us(SPI_DELAY); ax_sck_L(); sleep_us(SPI_DELAY);
	}

	// End of transmission
	ax_sel_H(); ax_mosi_L();	sleep_us(SPI_DELAY);
#endif

#ifdef ROBC_VER_3
	ax_rsel_L();
	// Octet 1
	SPI2->DR = ((write) ? 0x80 : 0) | 0x70 | ((adr >> 8) & 0x0F);
	while (!(SPI2->SR & SPI_SR_TXE))
		;
	while (!(SPI2->SR & SPI_SR_RXNE))
		;
	ax_status = (((uint16_t) SPI2->DR) << 8) & 0x7F00;

	// Octet 1
	SPI2->DR = adr & 0xFF;
	while (!(SPI2->SR & SPI_SR_TXE))
		;
	while (!(SPI2->SR & SPI_SR_RXNE))
		;
	ax_status |= SPI2->DR;

	// Octet 3
	SPI2->DR = (write) ? data : 0;
	while (!(SPI2->SR & SPI_SR_TXE))
		;
	while (!(SPI2->SR & SPI_SR_RXNE))
		;
	a = SPI2->DR;
	ax_rsel_H();
#endif

	return(a);
}


/* The AX5043 radio N-byte Rd/Wr access */
void ax_rw_N(uint8_t write, uint8_t adr, uint8_t *data, uint16_t num)
{

#ifdef ROBC_VER_2
	int8_t a, d;
	uint16_t i, n, m = 1;

	num = (num << 3) + 8;
	ax_sel_L();
	sleep_us(SPI_DELAY);
	
	for (i = 0; i < num; i++) {
		// MOSI control
		if (i == 0) {
			if (write != 0) ax_mosi_H(); else ax_mosi_L();
		}
		if (i == 1) n = 0x40;
		if (i >= 1 && i < 8) {
			if (adr & n) ax_mosi_H(); else ax_mosi_L();
			n >>= 1;
		}
		if (i > 0 && !(i % 8)) { n = 0x80; d = (write) ? *data : 0; }
		if (i >= 8) {
			if (d & n) ax_mosi_H(); else ax_mosi_L();
			n >>= 1;
		}
		sleep_us(SPI_DELAY);

		// MISO read
		if (i == 1) { m = 0x4000; /*ax_status = 0; TEST !!!! */ ax_status &= 0x00FF;}
		if (i >= 1 && i < 8) {
			if (ax_miso()) ax_status |= m;
			m >>= 1;
		}
		if (i > 0 && !(i % 8)) { m = 0x80; a = 0; }
		if (i >= 8) {
			if (ax_miso()) a |= m;
			m >>= 1;
		}
		if (m == 0) {
			if (write == 0) *data = a;
			data++;
		}

		// SCK control
		ax_sck_H(); sleep_us(SPI_DELAY); ax_sck_L(); sleep_us(SPI_DELAY);
	}

	// End of transmission
	ax_sel_H(); ax_mosi_L();	sleep_us(SPI_DELAY);
#endif

#ifdef ROBC_VER_3
	uint8_t i;

	ax_rsel_L();
	// Octet 1
	SPI2->DR = ((write) ? 0x80 : 0) | (adr & 0x7F);
	while (!(SPI2->SR & SPI_SR_TXE))
		;
	while (!(SPI2->SR & SPI_SR_RXNE))
		;
	//ax_status &= 0x7F00; TEST !!!!
	ax_status &= 0x00FF;
	ax_status |= (((uint16_t) SPI2->DR) << 8) & 0x7F00;
	
	// Octet 2..N
	for (i = 0; i < num; i++) {
		SPI2->DR = (write) ? *data : 0;
		while (!(SPI2->SR & SPI_SR_TXE))
			;
		while (!(SPI2->SR & SPI_SR_RXNE))
			;
		if (write) SPI2->DR; else *data = SPI2->DR;
		data++;
	}
	ax_rsel_H();
#endif
	
	return;
}


/* Default AX5043 configuration */
void ax_init(void)
{
	uint32_t n;

	// PIN PA control lines - PAEN, LOW = PA8, PA12
	GPIOA->CRH &= CONFMASK(0); GPIOA->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 0);
	GPIOA->CRH &= CONFMASK(4); GPIOA->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 4);
	GPIOA->BRR = (uint32_t) (0x00000001L << 8);  // PAEN = 0 (PA OFF)
	GPIOA->BSRR = (uint32_t) (0x00000001L << 12);  // LOW = 1 (Power high)
	
#ifdef ROBC_VER_2
	// Radio software SPI - RSEL, RIRQ, RMISO, RMOSI, RSCK = PB8, PB9, PB10, PB11, PB12
	GPIOB->CRH &= CONFMASK(0); GPIOB->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 0);
	GPIOB->CRH &= CONFMASK(1); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_PULL, 1);  // the safest state
	GPIOB->CRH &= CONFMASK(2); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_FLOAT, 2);
	GPIOB->CRH &= CONFMASK(3); GPIOB->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 3);
	GPIOB->CRH &= CONFMASK(4); GPIOB->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 4);
	// Default radio SPI pin state
	GPIOB->BSRR = (uint32_t) (0x00000001L << 8);  // RSEL = 1
	GPIOB->BRR  = (uint32_t) (0x00000001L << 11);  // RMOSI = 0
	GPIOB->BRR  = (uint32_t) (0x00000001L << 12);  // RSCK = 0
	// Radio data pins - RDCL, RDDA
	GPIOB->CRH &= CONFMASK(5); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_PULL, 5);
	GPIOB->CRH &= CONFMASK(6); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_PULL, 6);
#endif

#ifdef ROBC_VER_3
	// PIN Radio interface - RIRQ, RDCL, RDDA, RSEL, RSCK, RMISO, RMOSI = PB9, PB10, PB11, PB12, PB13, PB14, PB15
	GPIOB->CRH &= CONFMASK(1); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_PULL, 1); // the safest state
	GPIOB->CRH &= CONFMASK(2); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_PULL, 2); // the safest state
	GPIOB->CRH &= CONFMASK(3); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_PULL, 3); // the safest state
//	GPIOB->CRH &= CONFMASK(2); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_FLOAT, 2);
//	GPIOB->CRH &= CONFMASK(3); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_OUT_PP, 3);
	GPIOB->CRH &= CONFMASK(4); GPIOB->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 4);
	GPIOB->CRH &= CONFMASK(5); GPIOB->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_AFIO_PP, 5);
	GPIOB->CRH &= CONFMASK(6); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_FLOAT, 6);
	GPIOB->CRH &= CONFMASK(7); GPIOB->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_AFIO_PP, 7);
	GPIOB->BSRR = (uint32_t) (0x00000001L << 12);  // RSEL = 1

	// Reset SPI2 peripherial
	RCC->APB1RSTR = RCC_APB1RSTR_SPI2RST;
	for (n = 0; n < 255; n++) __nop();
	RCC->APB1RSTR = 0;

	// SPI2 - PilsenCUBE radio interface
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_SPE | ((RADIO_SPI_BAUD_RATE & 0x7) << 3);
	SPI2->CR2 = SPI_CR2_SSOE;
#endif

	// AX5043 reset
	ax_rw_2(1, 0x02, 0xE0);
	for (n = 0; n < 255; n++) __nop();
	ax_rw_2(1, 0x02, 0x60);  // 0x04 or 0x60 for external XO?
	for (n = 0; n < 255; n++) __nop();

	// Ax5043 register read and write test
	n = 1;
	if (ax_rw_3(0, 0x00, 0x00) == 0x51) ax_startup |= n;
	n <<= 1;
	if (ax_rw_3(0, 0x01, 0x00) == 0xC5) ax_startup |= n;
	n <<= 1;
	ax_rw_2(1, 0x01, 0xAA);
	if (ax_rw_3(0, 0x01, 0x00) == 0xAA) ax_startup |= n;
	n <<= 1;
	ax_rw_3(1, 0x164, 0x06);  // Single ended transmitter
	ax_rw_2(1, 0x26, 0x06);  // PWRAMP pin inverted PA control
	ax_rw_2(1, 0x27, 0x00);  // PWRAMP - PA off
	ax_startup |= ax_rw_2(0, 0x03, 0x00) << 8;  // power status register read

	// performance tuning registers initialization
	ax_rw_3(1, 0xF00, rcon_ptrg_ram[0]);
	ax_rw_3(1, 0xF0C, rcon_ptrg_ram[1]);
	ax_rw_3(1, 0xF0D, rcon_ptrg_ram[2]);
	ax_rw_3(1, 0xF10, rcon_ptrg_ram[3]);
	ax_rw_3(1, 0xF11, rcon_ptrg_ram[4]);
	ax_rw_3(1, 0xF1C, rcon_ptrg_ram[5]);
	ax_rw_3(1, 0xF21, rcon_ptrg_ram[6]);
	ax_rw_3(1, 0xF22, rcon_ptrg_ram[7]);
	ax_rw_3(1, 0xF23, rcon_ptrg_ram[8]);
	ax_rw_3(1, 0xF26, rcon_ptrg_ram[9]);
	ax_rw_3(1, 0xF34, rcon_ptrg_ram[10]);
	ax_rw_3(1, 0xF35, rcon_ptrg_ram[11]);
	ax_rw_3(1, 0xF44, rcon_ptrg_ram[12]);
	ax_rw_3(1, 0xF72, rcon_ptrg_ram[13]);

	// baseband tuning initialization
	ax_rw_3(1, 0x188, rcon_ptrg_ram[14]);
	ax_rw_3(1, 0x189, rcon_ptrg_ram[15]);

	// external interruput on PB9 (RIRQ) prepare
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	EXTI->PR |= 1 << 9;  // clear pending flag
	AFIO->EXTICR[2] &= ~(0x0F << 4);
	AFIO->EXTICR[2] |= 1 << 4;  // PB9 onto EXTI9
	EXTI->IMR |= 1 << 9;  // EXTI8 enable
	EXTI->RTSR |= 1 << 9;  // rising edge detector on EXTI8
	NVIC_EnableIRQ(EXTI9_5_IRQn);	

	// analog debug output
#ifdef _DEBUG_ANALOG_
	//ax_rw_3(1, 0x332, 0x03);  // output TRKFREQUENCY
	ax_rw_3(1, 0x332, 0x07);  // output RSSI, A bullshit in datasheet!
	ax_rw_3(1, 0x330, 0x00);  // DACVALUE = 0
	ax_rw_3(1, 0x331, 0x0C);  // DACSHIFT = 12 bit
	//ax_rw_3(1, 0x331, 0x08);  // DACSHIFT = 8 bit, TRKFRQ.. zoom
	ax_rw_2(1, 0x25, 0x05);  // ANTSEL as output
	//ax_rw_2(1, 0x26, 0x05);  // PWRAMP as output
#endif

	// digital debug output
#ifdef _DEBUG_DIGITAL_
	ax_rw_2(1, 0x22, 0x04);  // DCLK -> modem clock output, more variants!
	ax_rw_2(1, 0x23, 0x07);  // DATA -> modem data output a LOT of variants! (0x07 - raw data, NRZI decoded, no descrabled)
#endif

	return;
}


/* The AX5043 radio synthesizer frequency setting */
void ax_frequency(uint8_t vfo, uint32_t frq, uint8_t vcoran)
{
	uint64_t x;
	uint8_t adr;
	
	if (!vfo) {
		// FREQA register set will be used, selected in 0x30 register, internal PLL filter
		adr = 0x37; ax_rw_2(1, 0x30, 0x09);
	}	else {
		// FREQB register set will be used, selected in 0x30 register, internal PLL filter
		adr = 0x3F; ax_rw_2(1, 0x30, 0x89);
	}

	ax_rw_2(1, 0x31, 0x08); // Default charge pump current
	ax_rw_2(1, 0x32, 0x04); // fpd = fxtal, RF prescaler =2
	x = (uint64_t) frq * (uint64_t) 0x1000000L / (uint64_t) FRQ_XTAL;
#ifdef FRQ_ERR
	x = x * (uint64_t) 1000000000L / ((uint64_t) RF_FRQ_ERR + (uint64_t) 1000000000L);
#endif

	// Frequency register setting
	ax_rw_2(1, adr, (x & 0xFF) | 0x01); // +1/2
	x >>= 8; adr--;
	ax_rw_2(1, adr, x & 0xFF);
	x >>= 8; adr--;
	ax_rw_2(1, adr, x & 0xFF);
	x >>= 8; adr--;
	ax_rw_2(1, adr, x & 0xFF);

	// VCO autoranging
	if (vcoran) {
		ax_rw_2(1, 0x33, 0x10); 
		while ((x = ax_rw_2(0, 0x33, 0x00)) & 0x10)
			;
	} else {
		// No autoranging, PLL lock test only
		// Should be handled with a timeout!!! (If it fails, use autoranging again.)
		while ((x = ax_rw_2(0, 0x33, 0x00)) & 0x40)
			;
	}

	// Save VCO & PLL status
	ax_startup |= x << 16;
	return;
}


/* The AX5043 radio synthesizer VFO selection */
void ax_vfo(uint8_t vfo)
{
	ax_rw_2(1, 0x30, 0x09 | (vfo == 1) ? 0x80 : 0x00);
	return;
}


/* Reset and setup the AX5043 FIFO */
void ax_fifo_init(void)
{
	ax_rw_2(1, 0x28, FIFOCMD_CLR_DFL);
	ax_rw_2(1, 0x28, FIFOCMD_CLR_ERR);
	ax_rw_2(1, 0x2E, 0);
	ax_rw_2(1, 0x2F, FIFO_FREE_THRESHOLD); // safe thdreshold for writes to FIFO
	return;
}


/* Initialize the AX5043 CRC generator */
void ax_crc_init(void)
{
	static uint8_t crc_init[] = {
		AX_CRC_INIT >> 24,
		(AX_CRC_INIT >> 16) & 0xFF,
		(AX_CRC_INIT >> 8) & 0xFF,
		AX_CRC_INIT & 0xFF
	};
	
	ax_rw_N(1, 0x14, crc_init, 4);
	return;
}


/* Sets the AX5043 radio as continuous FM transmitter */
void ax_mode_fm(void)
{
	ax_rw_2(1, 0x27, 0x00);  // PWRAMP - PA off
	ax_rw_2(1, 0x10, 0x0B);  // FM

	ax_rw_3(1, 0x161, rcon_fmtx_ram[0]); // fskdev2
	ax_rw_3(1, 0x162, rcon_fmtx_ram[1]); // fskdev1
	ax_rw_3(1, 0x163, rcon_fmtx_ram[2]); // fskdev0, dev = +/-fxtal / 2^(fskfev0[2:0] + 1) => min. zdvih 62.5 kHz - DEFINE!!

	ax_rw_3(1, 0x301, 0x05); // gpadcperiod, sr = fxtal / (32 * gpadcperiod) => 100 kHz
	ax_rw_3(1, 0x300, 0x06); // gpadcctrl, continuous sampling, gpadc13=1 => Modulation input is GPADC1 and GPADC2 differentially
	ax_rw_2(1, 0x23, 0x04);  // pinfuncdata, undocumented code to switch TX on permanently
	ax_pwrmode(AX_PWRMODE_TX);
	ax_rw_2(1, 0x27, 0x01);  // PWRAMP - PA on
	ax_startup |= ax_rw_2(0, 0x03, 0x00) << 8;  // power status
	return;
}


/* Sets the AX5043 radio as wire mode ASK transmitter */
void ax_mode_askw(void)
{
	ax_rw_2(1, 0x27, 0x00); // PWRAMP - PA off
	ax_rw_2(1, 0x10, 0x00); // ASK
	ax_rw_2(1, 0x11, 0x00); // No differential encoding
	ax_rw_3(1, 0x165, 0x00); // txrate2
	ax_rw_3(1, 0x166, 0x00); // txrate1
	ax_rw_3(1, 0x167, 0x00); // txrate0
	//ax_rw_3(1, 0x167, 0x11); // rxrate0 => DEFINEd in morse.h, used in morse_init
	// TXRATE = (WPM * 2^24) / (2.4 * fxtal)
	ax_rw_2(1, 0x23, 0x04); // pinfuncdata, wire mode?
	ax_rw_2(1, 0x22, 0x05); // pinfuncdata, wire mode?
	ax_rw_3(1, 0x164, 0x02); // Single ended transmitter, no amplitude shaping
	ax_startup |= ax_rw_2(0, 0x03, 0x00) << 8;  // power status
	return;
}


/* Sets the AX5043 radio as AFSK FIFO transceiver */
void ax_mode_afsk(uint8_t crc_mode)
{
	ax_rw_2(1, 0x27, 0x00); // PWRAMP - PA off
	ax_rw_2(1, 0x10, 0x0A); // AFSK
	ax_rw_2(1, 0x11, 0x03); // encoding, NRZI, scrambler is off
#ifdef _TONE_DEBUG_
	ax_rw_2(1, 0x11, 0x00); // encoding off for tone debug
	ax_rw_2(1, 0x12, 0x00); // no framing for RAW data
#endif
	ax_rw_2(1, 0x12, 0x04 | (crc_mode & 0x07) << 4);  // HDLC framing, CRC mode
	// TXRATE = (bitrate / fxtal * 2^24) + 1 -> put 1200 Bd
	ax_rw_3(1, 0x165, rcon_aftx_ram[0]); // txrate2
	ax_rw_3(1, 0x166, rcon_aftx_ram[1]); // txrate1
	ax_rw_3(1, 0x167, rcon_aftx_ram[2]); // txrate0
	// AFSK 1200 Hz / 2200 Hz
	ax_rw_3(1, 0x110, rcon_aftx_ram[3]); // afskspace1
	ax_rw_3(1, 0x111, rcon_aftx_ram[4]); // afskspace0
	ax_rw_3(1, 0x112, rcon_aftx_ram[5]); // afskmark1
	ax_rw_3(1, 0x113, rcon_aftx_ram[6]); // afskmark0
	ax_rw_3(1, 0x164, 0x06); // single ended transmitter, amplitude shaping
	// FM deviation 3 or 5 kHz peak? Where to set?
	ax_startup |= ax_rw_2(0, 0x03, 0x00) << 8;  // power status
	return;
}


/* Sets the AX5043 radio as GMSK G3RUH FIFO transceiver */
void ax_mode_g3ruh(uint8_t type, uint8_t crc_mode, uint8_t encoding)
{
	uint32_t txrate;
	
	switch (type) {
		case G3RUH_1200:  txrate = rcon_gdtx_prog[0]; break;
		case G3RUH_2400:  txrate = rcon_gdtx_prog[1]; break;
		case G3RUH_4800:  txrate = rcon_gdtx_prog[2]; break;
		case G3RUH_9600:  txrate = rcon_gdtx_prog[3]; break;
		case G3RUH_19200: txrate = rcon_gdtx_prog[4]; break;
		default:          txrate = rcon_gdtx_ram;
	}
	txrate++;

	ax_rw_2(1, 0x27, 0x00);  // PWRAMP - PA off
	ax_rw_2(1, 0x10, 0x08);  // FSK, because MSK is not working
  ax_rw_2(1, 0x11, encoding);  // encoding, NRZI, scrambler
	
	ax_rw_2(1, 0x12, 0x04 | (crc_mode & 0x07) << 4);  // HDLC framing, CRC mode
	// TXRATE = (bitrate / fxtal * 2^24) -> put 1200 Bd
	ax_rw_3(1, 0x167, txrate & 0xFF);  // TXRATE0	
	ax_rw_3(1, 0x166, (txrate >> 8) & 0xFF);  // TXRATE1
	ax_rw_3(1, 0x165, 0);  // TXRATE2
	// FSK deviation, don't set automatically
	txrate >>= 3; txrate++;  // from the AX config tool
	ax_rw_3(1, 0x163, txrate & 0xFF);  // FSKDEV0
	ax_rw_3(1, 0x162, (txrate >> 8) & 0xFF);  // FSKDEV1	
	ax_rw_3(1, 0x161, 0);  // FSKDEV2
	// transmitter setup
	ax_rw_3(1, 0x160, rcon_gdtx_prog[5]); // frequency shaping
	ax_rw_3(1, 0x164, 0x06);  // single ended transmitter, amplitude shaping

	// receiver - packet format
	ax_rw_3(1, 0x200, 0x01);  // data lsb first
	ax_rw_3(1, 0x201, 0x80);  // length config
	ax_rw_3(1, 0x202, 0x01);  // packet length offset!
	ax_rw_3(1, 0x203, 0xF0);  // max length

	// receiver - packet controller
	ax_rw_3(1, 0x220, 0x33);
	ax_rw_3(1, 0x221, 0x14);
	ax_rw_3(1, 0x223, 0x33);
	ax_rw_3(1, 0x224, 0x14);
	ax_rw_3(1, 0x225, 0x00);
	ax_rw_3(1, 0x226, 0x73);
	ax_rw_3(1, 0x227, 0x00);
	ax_rw_3(1, 0x228, 0x03);
	ax_rw_3(1, 0x229, 0x00);  // pream 1 timeout
	ax_rw_3(1, 0x22A, 0x17);  // pream 2 timeout
	ax_rw_3(1, 0x22B, 0x00);  // pream 2 timeout
	ax_rw_3(1, 0x22C, 0xF8);
	ax_rw_3(1, 0x22F, 0x00);
	ax_rw_3(1, 0x230, 13);    // max chunk size 13 = 240 B !!!
	ax_rw_3(1, 0x231, 0x00);
	ax_rw_3(1, 0x232, 0x54);  // store flags ANT RSSI, RSSI, RF FOFFS

#ifdef _DEBUG_ACCEPT_
	#if _DEBUG_ACCEPT_ == ALL
	ax_rw_3(1, 0x233, 0x3F);  // accept as much you can
	#elif _DEBUG_ACCEPT_ == CRC
	ax_rw_3(1, 0x233, 0x04);  // accept wrong crc
	#elif _DEBUG_ACCEPT_ == CRCADRS
	ax_rw_3(1, 0x233, 0x1C);  // accept wrong crc and address and size
	#elif _DEBUG_ACCEPT_ == LARGE
	ax_rw_3(1, 0x233, 0x20);  // accept lrgp
	#elif _DEBUG_ACCEPT_ == ALLWOLARGE
	ax_rw_3(1, 0x233, 0x1F);	// accept all except long puckets
	ax_rw_3(1, 0x233, 0x08);  // experimental
	#endif
#else
	ax_rw_3(1, 0x233, 0x00);  // accept nothing special
#endif
	
	// receiver parameters
	switch (type) {
		case G3RUH_1200:
			// NOT DEFINED !!!
			break;
		case G3RUH_2400:
			// NOT DEFINED !!!
			break;
		case G3RUH_4800:
			// NOT DEFINED !!!
			break;
		case G3RUH_9600:
			ax_rw_3(1, 0x102, 0x0E); // DECIMATION
			ax_rw_3(1, 0x100, 0x03); // Fif, MSB
			ax_rw_3(1, 0x101, 0x01); // Fif, LSB
			break;
		case G3RUH_19200:
			ax_rw_3(1, 0x102, 0x07); // DECIMATION
			ax_rw_3(1, 0x100, 0x06); // Fif, MSB
			ax_rw_3(1, 0x101, 0x02); // Fif, LSB
			break;
		default:
			// NOT DEFINED !!!
			break;
	}

	ax_rw_3(1, 0x103, 0x00); // DATA RATE, MSB - bitrate independent
	ax_rw_3(1, 0x104, 0x3C); // DATA RATE
	ax_rw_3(1, 0x105, 0xE4); // DATA RATE, LSB
	ax_rw_3(1, 0x106, 0x00); // MAXDROFFSET, MSB - don't use if RX/TX timing error is less than 0.15%
	ax_rw_3(1, 0x107, 0x00); // MAXDROFFSET
	ax_rw_3(1, 0x108, 0x00); // MAXDROFFSET, LSB
	ax_rw_3(1, 0x109, 0x80); // MAXRFOFFSET | 0x80 - track at LO1
	ax_rw_3(1, 0x10A, 0x04); // 04 (~+/-1 kHz) MAXRFOFFSET
	ax_rw_3(1, 0x10B, 0x90); // 90 (~+/-1 kHz) MAXRFOFFSET, LSB (!! 0x00FF works surprisingly good, 2% but still V)
	ax_rw_3(1, 0x10C, 0x00); // 00 FSKDMAX1
	ax_rw_3(1, 0x10D, 0xA6); // A6 FSKDMAX0 - bitrate independent
	ax_rw_3(1, 0x10E, 0xFF); // FF FSKDMIN1
	ax_rw_3(1, 0x10F, 0x5A); // 5A FSKDMIN0 - bitrate independent
	ax_rw_3(1, 0x116, 0x00); // baseband AFC loop leakage, default 0

	// receiver parameter set config
	ax_rw_3(1, 0x117, 0xF4); // dets 0, 1 and 3
#ifdef _DEBUG_RECSET_
	ax_rw_3(1, 0x117, 0x00); // dets 0 only - debug only
#endif

	// receiver paramater set 0
	ax_rw_3(1, 0x120, 0xB5); // AGCGAIN0 - bitrate dependent
	ax_rw_3(1, 0x121, 0x84); // AGCTARGET0 - bitrate independent
	ax_rw_3(1, 0x124, 0xF8); // TIMEGAIN0, default 0xF8
	ax_rw_3(1, 0x125, 0xF2); // DRGAIN0, default 0xF2
	ax_rw_3(1, 0x126, 0xC3); // PHASEGAIN0, default 0xC3
	ax_rw_3(1, 0x127, 0x0F); // FREQGAINA0, off
	ax_rw_3(1, 0x128, 0x1F); // FREQGAINB0, off
	ax_rw_3(1, 0x129, 0x09); // 09 FREQGAINC0 - bitrate dependent
	ax_rw_3(1, 0x12A, 0x09); // 09 FREQGAIND0 - bitrate dependent
	ax_rw_3(1, 0x12B, 0x06); // AMPLITUDEGAIN0, default 0x46
	ax_rw_3(1, 0x12C, 0x00); // FREQDEV10, default 0
	ax_rw_3(1, 0x12D, 0x00); // FREQDEV00, default 0
	ax_rw_3(1, 0x12E, 0x16); // FOURFSK0, default 16
	ax_rw_3(1, 0x12F, 0x00); // BBOFSRES0, default 0x88

	// receiver paramater set 1
	ax_rw_3(1, 0x130, 0xB5); // AGCGAIN1 - bitrate dependent
	ax_rw_3(1, 0x131, 0x84); // AGCTARGET1 - bitrate independent
	ax_rw_3(1, 0x134, 0xF6); // TIMEGAIN1, default 0xF8
	ax_rw_3(1, 0x135, 0xF1); // DRGAIN1, default 0xF2
	ax_rw_3(1, 0x136, 0xC3); // PHASEGAIN1, default 0xC3
	ax_rw_3(1, 0x137, 0x0F); // FREQGAINA1, off
	ax_rw_3(1, 0x138, 0x1F); // FREQGAINB1, off
	ax_rw_3(1, 0x139, 0x0E); // 09 FREQGAINC1 - bitrate dependent
	ax_rw_3(1, 0x13A, 0x0E); // 09 FREQGAIND1 - bitrate dependent
	ax_rw_3(1, 0x13B, 0x06); // AMPLITUDEGAIN1, default 0x46
	ax_rw_3(1, 0x13C, 0x00); // FREQDEV11, default 0
	ax_rw_3(1, 0x13D, 0x32); // FREQDEV01, default 0
	ax_rw_3(1, 0x13E, 0x16); // FOURFSK1, default 16
	ax_rw_3(1, 0x13F, 0x00); // BBOFSRES1, default 0x88

	// receiver paramater set 3
	ax_rw_3(1, 0x150, 0xFF); // AGCGAIN3 - bitrate independent
	ax_rw_3(1, 0x151, 0x84); // AGCTARGET3 - bitrate independent
	ax_rw_3(1, 0x154, 0xF5); // TIMEGAIN3, default 0xF8
	ax_rw_3(1, 0x155, 0xF0); // DRGAIN3, default 0xF2
	ax_rw_3(1, 0x156, 0xC3); // PHASEGAIN3, default 0xC3
	ax_rw_3(1, 0x157, 0x0F); // FREQGAINA3, off
	ax_rw_3(1, 0x158, 0x1F); // FREQGAINB3, off
	ax_rw_3(1, 0x159, 0x0D); // 0D FREQGAINC3 - bitrate dependent
	ax_rw_3(1, 0x15A, 0x0D); // 0D FREQGAIND3 - bitrate dependent
	ax_rw_3(1, 0x15B, 0x06); // AMPLITUDEGAIN3, default 0x46
	ax_rw_3(1, 0x15C, 0x00); // FREQDEV13, default 0
	ax_rw_3(1, 0x15D, 0x32); // FREQDEV03, default 0
	ax_rw_3(1, 0x15E, 0x16); // FOURFSK3, default 16
	ax_rw_3(1, 0x15F, 0x00); // BBOFSRES3, default 0x88

	// pattern match
	ax_rw_3(1, 0x210, 0xAA); // AA MATCH0PAT3
	ax_rw_3(1, 0x211, 0xCC); // CC MATCH0PAT2
	ax_rw_3(1, 0x212, 0xAA); // AA MATCH0PAT1
	ax_rw_3(1, 0x213, 0xCC); // CC MATCH0PAT0
	ax_rw_3(1, 0x214, 0x00); // MATCH0LEN
	ax_rw_3(1, 0x215, 0x00); // MATCH0MIN
	ax_rw_3(1, 0x216, 0x1F); // MATCH0MAX
	ax_rw_3(1, 0x218, 0x7E); // MATCH1PAT1 - bit reversed preamble!
	ax_rw_3(1, 0x219, 0x7E); // MATCH1PAT0 - bit reversed preamble!
	ax_rw_3(1, 0x21C, 0x0A); // MATCH1LEN - 0x0A without FEC, 0x8A with FEC !!!
	ax_rw_3(1, 0x21D, 0x00); // MATCH1MIN
	ax_rw_3(1, 0x21E, 0x0A); // MATCH1MAX

	// irq
	ax_rw_2(1, 0x24, 0x03); // PINFUNCIRQ - IRQ output
//ax_rw_2(1, 0x07, 0x40); // Radio controller event
	ax_rw_2(1, 0x07, 0x01); // FIFO not empty
	ax_rw_2(1, 0x09, 0x04); // RADIOEVENTMASK
	
	ax_startup |= ax_rw_2(0, 0x03, 0x00) << 8;  // power status
	return;
}


/* Set the power mode of AX5043 to RX, TX, ... */
void ax_pwrmode(uint8_t mode)
{
	ax_rw_2(1, 0x02, 0x60 | (mode & 0x0F));
	return;
}


/*! @} */
