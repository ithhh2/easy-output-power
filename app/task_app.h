/*
 * app.h
 *
 *  Created on: Nov 14, 2025
 *      Author: lceda
 */

#ifndef TASK_APP_H_PP_H_
#define TASK_APP_H_PP_H_

#include "main.h"
#include "hw_key.h"

#define VOL_PAGE 		0x01
#define CUR_PAGE 		0x02
#define POWER_PAGE 		0x03

#define OPENPOWER		0x20
#define CLOSEPOWER		0x21

enum Key_e
{
	SET_KEY = 0,
	ADD_KEY,
	SUB_KEY,
	KEY_NUM
};

enum Relay_e
{
	Relay_1V8 = 0,
	Relay_Vbus,
	Relay_3V3,
	Relay_Num
};

struct SystemParam
{
	uint8_t isOpen;
	uint8_t currentPage;
	uint8_t setVolValue;
	uint16_t protectValue;
	uint16_t curValue;
	float volValue;
};

extern KEY_HandleDef key[KEY_NUM];

void init_task(void);
void trigger_scanf(void);
void key_handle(KEY_HandleDef *key_handle);
void protectiveScan(void);

#endif /* TASK_APP_H_ */
