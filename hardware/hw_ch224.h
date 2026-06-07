/*
 * hw_ch224.h
 *
 *  Created on: Nov 14, 2025
 *      Author: lceda
 */

#ifndef HW_CH224_H_
#define HW_CH224_H_

#include "main.h"

typedef struct
{
	GPIO_TypeDef *cfg1_gpio;
	uint32_t cfg1_pin;
	GPIO_TypeDef *cfg2_gpio;
	uint32_t cfg2_pin;
	GPIO_TypeDef *cfg3_gpio;
	uint32_t cfg3_pin;
	uint8_t deception_vol;			//ch224k deception voltage
}CH224K_HandleDef;

#define VOL_1V8			0x10
#define VOL_3V3			0x11
#define VOL_5V0			0x12
#define VOL_9V0			0x13
#define VOL_12V			0x14

void set_ch224k_deceptionVol(CH224K_HandleDef *ch224k_t);
CH224K_HandleDef ch224k_init(void);

#endif /* HW_CH224_H_ */
