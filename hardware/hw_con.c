/*
 * hw_con.c
 *
 *  Created on: Nov 4, 2025
 *      Author: lceda
 */

#include "hw_con.h"

/*
 * Function Content: turn on relay and will only activate one relay
 * Function Parameter: CON_HandleDef con_t
 * Return Value: No
 */
void control_relayx(Relay_HandleDef *relay_t)
{
	if (relay_t == NULL)
	{
		return;
	}

	relay_close_all();
	if(relay_t->relay_open_level == 0){
		HAL_GPIO_WritePin(relay_t->relay_gpio,relay_t->relay_pin,GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(relay_t->relay_gpio,relay_t->relay_pin,GPIO_PIN_SET);
	}
}

/*
 * Function Content: assigning relay pins
 * Function Parameter: relay gpio、pin and open level
 * Return Value: CON_HandleDef
 */
Relay_HandleDef relay_init(GPIO_TypeDef *relay_gpio,uint32_t relay_pin,uint8_t relay_open_level)
{
	Relay_HandleDef relay_t;
	relay_t.relay_gpio = relay_gpio;
	relay_t.relay_pin = relay_pin;
	relay_t.relay_open_level = relay_open_level;
	return relay_t;
}

/*
 * Function Content: close all relay
 * Function Parameter: No
 * Return Value: No
 */
void relay_close_all(void)
{
	HAL_GPIO_WritePin(CON_IO1_GPIO_Port,CON_IO1_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(CON_IO2_GPIO_Port,CON_IO2_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(CON_IO3_GPIO_Port,CON_IO3_Pin,GPIO_PIN_SET);
}
