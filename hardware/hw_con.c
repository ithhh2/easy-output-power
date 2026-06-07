/*
 * hw_con.c
 *
 *  Created on: Nov 4, 2025
 *      Author: lceda
 */

#include "hw_con.h"

#include "main.h"

static Relay_HandleDef *active_relay = NULL;

static void relay_drive(Relay_HandleDef *relay_t, uint8_t active)
{
	if (relay_t == NULL)
	{
		return;
	}

	if (active != 0U)
	{
		if (relay_t->relay_open_level == 0U)
		{
			HAL_GPIO_WritePin(relay_t->relay_gpio, relay_t->relay_pin, GPIO_PIN_RESET);
		}
		else
		{
			HAL_GPIO_WritePin(relay_t->relay_gpio, relay_t->relay_pin, GPIO_PIN_SET);
		}
	}
	else
	{
		HAL_GPIO_WritePin(relay_t->relay_gpio, relay_t->relay_pin, GPIO_PIN_SET);
	}
}

static void relay_close_other_pins(Relay_HandleDef *keep_relay)
{
	if ((keep_relay == NULL) ||
	    (keep_relay->relay_gpio != CON_IO1_GPIO_Port) ||
	    (keep_relay->relay_pin != CON_IO1_Pin))
	{
		HAL_GPIO_WritePin(CON_IO1_GPIO_Port, CON_IO1_Pin, GPIO_PIN_SET);
	}

	if ((keep_relay == NULL) ||
	    (keep_relay->relay_gpio != CON_IO2_GPIO_Port) ||
	    (keep_relay->relay_pin != CON_IO2_Pin))
	{
		HAL_GPIO_WritePin(CON_IO2_GPIO_Port, CON_IO2_Pin, GPIO_PIN_SET);
	}

	if ((keep_relay == NULL) ||
	    (keep_relay->relay_gpio != CON_IO3_GPIO_Port) ||
	    (keep_relay->relay_pin != CON_IO3_Pin))
	{
		HAL_GPIO_WritePin(CON_IO3_GPIO_Port, CON_IO3_Pin, GPIO_PIN_SET);
	}
}

void control_relayx(Relay_HandleDef *relay_t)
{
	if (relay_t == NULL)
	{
		return;
	}

	if (relay_t == active_relay)
	{
		return;
	}

	relay_drive(relay_t, 1U);
	HAL_Delay(5);
	relay_close_other_pins(relay_t);
	active_relay = relay_t;
}

Relay_HandleDef relay_init(GPIO_TypeDef *relay_gpio, uint32_t relay_pin, uint8_t relay_open_level)
{
	Relay_HandleDef relay_t;

	relay_t.relay_gpio = relay_gpio;
	relay_t.relay_pin = relay_pin;
	relay_t.relay_open_level = relay_open_level;
	return relay_t;
}

void relay_close_all(void)
{
	relay_close_other_pins(NULL);
	active_relay = NULL;
}
