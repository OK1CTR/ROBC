#ifndef STM32F10X_GPIO_H
#define STM32F10X_GPIO_H

#define GPIO_M_INPUT  0x00000000ul
#define GPIO_M_OUT10  0x00000001ul
#define GPIO_M_OUT02	0x00000002ul
#define GPIO_M_OUT50	0x00000003ul

#define GPIO_I_ANALOG 0x00000000ul
#define GPIO_I_FLOAT  0x00000001ul
#define GPIO_I_PULL   0x00000002ul

#define GPIO_OUT_PP   0x00000000ul
#define GPIO_OUT_OD   0x00000001ul
#define GPIO_AFIO_PP  0x00000002ul
#define GPIO_AFIO_OD  0x00000003ul

#define GPIOCONF(mode, cnf, shift)	(((cnf << 2) | (mode)) << (shift << 2))
#define CONFMASK(shift) ((uint32_t)~(0x0000000Ful << (shift << 2)))

#endif
