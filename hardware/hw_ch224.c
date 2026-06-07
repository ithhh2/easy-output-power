/*
 * hw_ch224.c
 *
 *  Created on: Nov 14, 2025
 *      Author: lceda
 */
#include "hw_ch224.h"

/*
 * Function Content: set ch224k deception Voltage
 * Function Parameter: CH224K_HandleDef *ch224k_t
 * Return Value: No
 */
void set_ch224k_deceptionVol(CH224K_HandleDef *ch224k_t)
{
	if(ch224k_t->deception_vol == VOL_5V0){
		HAL_GPIO_WritePin(ch224k_t->cfg1_gpio,ch224k_t->cfg1_pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(ch224k_t->cfg2_gpio,ch224k_t->cfg2_pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ch224k_t->cfg3_gpio,ch224k_t->cfg3_pin,GPIO_PIN_RESET);
	}
	else if(ch224k_t->deception_vol == VOL_9V0){
		HAL_GPIO_WritePin(ch224k_t->cfg1_gpio,ch224k_t->cfg1_pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ch224k_t->cfg2_gpio,ch224k_t->cfg2_pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ch224k_t->cfg3_gpio,ch224k_t->cfg3_pin,GPIO_PIN_RESET);
	}
	else if(ch224k_t->deception_vol == VOL_12V){
		HAL_GPIO_WritePin(ch224k_t->cfg1_gpio,ch224k_t->cfg1_pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ch224k_t->cfg2_gpio,ch224k_t->cfg2_pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ch224k_t->cfg3_gpio,ch224k_t->cfg3_pin,GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(CFG1_GPIO_Port,CFG1_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(CFG2_GPIO_Port,CFG2_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(CFG3_GPIO_Port,CFG3_Pin,GPIO_PIN_RESET);
	}
}

/*
 * Function Content: assigning ch224k pins
 * Function Parameter: No
 * Return Value: CH224K_HandleDef
 */
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
