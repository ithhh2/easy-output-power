#ifndef __HW_KEY_H
#define __HW_KEY_H

#include "main.h"

//public macro define

#define DOUBLE_TIME 40  	//double click time -- 40ms
#define KEY_NoPress 0xFF	
#define KEY_ERROR   0x00
#define KEY_OK      0x01

#define KeyPress 0x10
#define KeyDoublePress 0x20 
#define KeyLongPress  0x30

typedef struct
{
	GPIO_TypeDef *key_gpio;
	uint16_t key_pin;
	uint8_t key_state;
	uint8_t key_press_level;
	uint8_t keyCnt;				//key pressed time interval count
	uint8_t keyFcnt; 			//double click time
	uint8_t keyCount; 		   	//key pressed count
	uint8_t keyLongFlag; 		//key long press flag
}KEY_HandleDef;

void key_scanf(KEY_HandleDef *key_handle);
KEY_HandleDef key_init(GPIO_TypeDef *key_gpio,uint16_t key_pin,uint8_t key_press_level);

#endif

