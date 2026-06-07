#include "hw_key.h"

KEY_HandleDef key_init(GPIO_TypeDef *key_gpio, uint16_t key_pin, uint8_t key_press_level)
{
	KEY_HandleDef key_handle = {0};

	key_handle.key_gpio = key_gpio;
	key_handle.key_pin = key_pin;
	key_handle.key_press_level = key_press_level;
	key_handle.key_state = KEY_NoPress;
	key_handle.key_phase = KEY_PHASE_IDLE;
	key_handle.key_cnt = 0U;
	return key_handle;
}

uint8_t key_is_held(const KEY_HandleDef *key_handle)
{
	if (key_handle == NULL)
	{
		return 0U;
	}

	return (key_handle->key_phase == KEY_PHASE_HELD) ? 1U : 0U;
}

void key_scanf(KEY_HandleDef *key_handle)
{
	uint8_t is_down = 0U;

	if (key_handle == NULL)
	{
		return;
	}

	key_handle->key_state = KEY_NoPress;
	is_down = (HAL_GPIO_ReadPin(key_handle->key_gpio, key_handle->key_pin) == key_handle->key_press_level) ?
	          1U :
	          0U;

	switch (key_handle->key_phase)
	{
	case KEY_PHASE_IDLE:
		if (is_down != 0U)
		{
			key_handle->key_phase = KEY_PHASE_DEBOUNCE;
			key_handle->key_cnt = 1U;
		}
		break;

	case KEY_PHASE_DEBOUNCE:
		if (is_down != 0U)
		{
			if (key_handle->key_cnt < 255U)
			{
				key_handle->key_cnt++;
			}

			if (key_handle->key_cnt >= KEY_DEBOUNCE_TICKS)
			{
				key_handle->key_phase = KEY_PHASE_HELD;
				key_handle->key_state = KeyPress;
			}
		}
		else
		{
			key_handle->key_phase = KEY_PHASE_IDLE;
			key_handle->key_cnt = 0U;
		}
		break;

	case KEY_PHASE_HELD:
	default:
		if (is_down != 0U)
		{
			break;
		}

		key_handle->key_phase = KEY_PHASE_IDLE;
		key_handle->key_cnt = 0U;
		break;
	}
}
