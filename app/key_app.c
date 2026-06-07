#include "key_app.h"

#include "app_config.h"
#include "app_state.h"
#include "task_app.h"
#include "ui_app.h"

static KEY_HandleDef key[KEY_NUM] = {0};
static uint32_t key_repeat_anchor_ms[KEY_NUM] = {0};
static uint32_t key_repeat_last_ms[KEY_NUM] = {0};

void key_app_init(void)
{
	key[SET_KEY] = key_init(SW1_GPIO_Port, SW1_Pin, GPIO_PIN_RESET);
	key[ADD_KEY] = key_init(SW2_GPIO_Port, SW2_Pin, GPIO_PIN_RESET);
	key[SUB_KEY] = key_init(SW3_GPIO_Port, SW3_Pin, GPIO_PIN_RESET);
}

static void key_repeat_reset(uint8_t key_index)
{
	key_repeat_anchor_ms[key_index] = 0U;
	key_repeat_last_ms[key_index] = 0U;
}

static void key_repeat_mark(uint8_t key_index)
{
	uint32_t now = HAL_GetTick();

	key_repeat_anchor_ms[key_index] = now;
	key_repeat_last_ms[key_index] = now;
}

static uint8_t key_repeat_due(uint8_t key_index)
{
	uint32_t now = HAL_GetTick();

	if (key_repeat_anchor_ms[key_index] == 0U)
	{
		return 0U;
	}

	if ((now - key_repeat_anchor_ms[key_index]) < KEY_REPEAT_DELAY_MS)
	{
		return 0U;
	}

	if ((now - key_repeat_last_ms[key_index]) < KEY_REPEAT_PERIOD_MS)
	{
		return 0U;
	}

	key_repeat_last_ms[key_index] = now;
	return 1U;
}

static void key_repeat_poll(void)
{
	AppState_t *state = app_state();

	if ((state->isOpen != DISABLE) ||
	    (state->currentPage != VOL_PAGE && state->currentPage != CUR_PAGE))
	{
		key_repeat_reset(ADD_KEY);
		key_repeat_reset(SUB_KEY);
		return;
	}

	if (key_is_held(&key[ADD_KEY]) == 0U)
	{
		key_repeat_reset(ADD_KEY);
	}
	else if (key_repeat_due(ADD_KEY) != 0U)
	{
		if (state->currentPage == VOL_PAGE)
		{
			task_adjust_voltage_up();
		}
		else
		{
			task_adjust_current_up();
		}
	}

	if (key_is_held(&key[SUB_KEY]) == 0U)
	{
		key_repeat_reset(SUB_KEY);
	}
	else if (key_repeat_due(SUB_KEY) != 0U)
	{
		if (state->currentPage == VOL_PAGE)
		{
			task_adjust_voltage_down();
		}
		else
		{
			task_adjust_current_down();
		}
	}
}

static void key_handle(KEY_HandleDef *key_handle)
{
	uint8_t key_index = KEY_NUM;
	AppState_t *state = app_state();

	if (key_handle->key_state != KeyPress)
	{
		return;
	}

	if (key_handle->key_pin == SW2_Pin)
	{
		key_index = ADD_KEY;
	}
	else if (key_handle->key_pin == SW3_Pin)
	{
		key_index = SUB_KEY;
	}

	switch (key_handle->key_pin)
	{
	case SW1_Pin:
		if (state->isOpen == DISABLE)
		{
			task_page_next();
		}
		break;

	case SW2_Pin:
		switch (state->currentPage)
		{
		case POWER_PAGE:
			task_toggle_power_output();
			break;
		case VOL_PAGE:
			if (state->isOpen == DISABLE)
			{
				task_adjust_voltage_up();
				key_repeat_mark(ADD_KEY);
			}
			break;
		case CUR_PAGE:
			if (state->isOpen == DISABLE)
			{
				task_adjust_current_up();
				key_repeat_mark(ADD_KEY);
			}
			break;
		default:
			break;
		}
		break;

	case SW3_Pin:
		switch (state->currentPage)
		{
		case POWER_PAGE:
			task_toggle_power_output();
			break;
		case VOL_PAGE:
			if (state->isOpen == DISABLE)
			{
				task_adjust_voltage_down();
				key_repeat_mark(SUB_KEY);
			}
			break;
		case CUR_PAGE:
			if (state->isOpen == DISABLE)
			{
				task_adjust_current_down();
				key_repeat_mark(SUB_KEY);
			}
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	if ((key_index != KEY_NUM) &&
	    (state->isOpen != DISABLE ||
	     (state->currentPage != VOL_PAGE &&
	      state->currentPage != CUR_PAGE)))
	{
		key_repeat_reset(key_index);
	}
}

void key_app_poll(void)
{
	uint8_t i = 0U;

	for (i = 0U; i < KEY_NUM; i++)
	{
		key_scanf(&key[i]);
		if (key[i].key_state == KeyPress)
		{
			key_handle(&key[i]);
		}
	}

	key_repeat_poll();
}
