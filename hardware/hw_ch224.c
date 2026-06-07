/*
 * hw_ch224.c
 *
 *  Created on: Nov 14, 2025
 *      Author: lceda
 */
#include "hw_ch224.h"

static void ch224_apply_5v0(CH224K_HandleDef *ch224k_t)
{
	HAL_GPIO_WritePin(ch224k_t->cfg1_gpio, ch224k_t->cfg1_pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ch224k_t->cfg2_gpio, ch224k_t->cfg2_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ch224k_t->cfg3_gpio, ch224k_t->cfg3_pin, GPIO_PIN_RESET);
}

void set_ch224k_deceptionVol(CH224K_HandleDef *ch224k_t)
{
	if (ch224k_t == NULL)
	{
		return;
	}

	if ((ch224k_t->deception_vol == VOL_5V0) ||
	    (ch224k_t->deception_vol == VOL_1V8) ||
	    (ch224k_t->deception_vol == VOL_3V3))
	{
		ch224_apply_5v0(ch224k_t);
	}
	else if (ch224k_t->deception_vol == VOL_9V0)
	{
		HAL_GPIO_WritePin(ch224k_t->cfg1_gpio, ch224k_t->cfg1_pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ch224k_t->cfg2_gpio, ch224k_t->cfg2_pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ch224k_t->cfg3_gpio, ch224k_t->cfg3_pin, GPIO_PIN_RESET);
	}
	else if (ch224k_t->deception_vol == VOL_12V)
	{
		HAL_GPIO_WritePin(ch224k_t->cfg1_gpio, ch224k_t->cfg1_pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ch224k_t->cfg2_gpio, ch224k_t->cfg2_pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ch224k_t->cfg3_gpio, ch224k_t->cfg3_pin, GPIO_PIN_SET);
	}
	else
	{
		ch224_apply_5v0(ch224k_t);
	}
}

CH224K_HandleDef ch224k_init(void)
{
	CH224K_HandleDef ch224k_handle;

	ch224k_handle.cfg1_gpio = CFG1_GPIO_Port;
	ch224k_handle.cfg1_pin = CFG1_Pin;
	ch224k_handle.cfg2_gpio = CFG2_GPIO_Port;
	ch224k_handle.cfg2_pin = CFG2_Pin;
	ch224k_handle.cfg3_gpio = CFG3_GPIO_Port;
	ch224k_handle.cfg3_pin = CFG3_Pin;
	ch224k_handle.deception_vol = VOL_5V0;
	return ch224k_handle;
}
