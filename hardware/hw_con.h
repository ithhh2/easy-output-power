/*
 * hw_con.h
 *
 *  Created on: Nov 4, 2025
 *      Author: lceda
 */

#ifndef HW_CON_H_
#define HW_CON_H_

#include "main.h"

typedef struct
{
	GPIO_TypeDef *relay_gpio;
	uint32_t relay_pin;
	uint8_t relay_open_level;
}Relay_HandleDef;

void control_relayx(Relay_HandleDef *relay_t);
Relay_HandleDef relay_init(GPIO_TypeDef *relay_gpio,uint32_t relay_pin,uint8_t relay_open_level);
void relay_close_all(void);

#endif /* HW_CON_H_ */
