#ifndef __HW_KEY_H
#define __HW_KEY_H

#include "main.h"

#define KEY_DEBOUNCE_TICKS     2U
#define KEY_REPEAT_DELAY_MS    1000U
#define KEY_REPEAT_PERIOD_MS   100U

#define KEY_NoPress 0xFF
#define KeyPress    0x10

#define KEY_PHASE_IDLE      0U
#define KEY_PHASE_DEBOUNCE  1U
#define KEY_PHASE_HELD      2U

typedef struct
{
	GPIO_TypeDef *key_gpio;
	uint16_t key_pin;
	uint8_t key_state;
	uint8_t key_press_level;
	uint8_t key_phase;
	uint8_t key_cnt;
} KEY_HandleDef;

void key_scanf(KEY_HandleDef *key_handle);
uint8_t key_is_held(const KEY_HandleDef *key_handle);
KEY_HandleDef key_init(GPIO_TypeDef *key_gpio, uint16_t key_pin, uint8_t key_press_level);

#endif
