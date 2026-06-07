#include "task_app.h"

#include "app_config.h"
#include "app_state.h"
#include "hw_adc_sample.h"
#include "hw_ch224.h"
#include "hw_con.h"
#include "iwdg_user.h"
#include "key_app.h"
#include "mid_oled.h"
#include "power_profile.h"
#include "protect_app.h"
#include "ui_app.h"

static CH224K_HandleDef ch224k_t = {0};
static Relay_HandleDef relay_t[3] = {0};

static uint32_t last_tick_ms = 0U;
static uint32_t last_display_ms = 0U;

static Relay_HandleDef *relay_from_id(uint8_t relay_id)
{
	switch (relay_id)
	{
	case RELAY_ID_1V8:
		return &relay_t[0];
	case RELAY_ID_3V3:
		return &relay_t[2];
	case RELAY_ID_VBUS:
	default:
		return &relay_t[1];
	}
}

static void open_power_switch(uint8_t tier_index)
{
	const PowerTier_t *tier = power_profile_get(tier_index);
	AppState_t *state = app_state();

	if (state->faultLatched != 0U)
	{
		return;
	}

	ch224k_t.deception_vol = tier->ch224_vol;
	set_ch224k_deceptionVol(&ch224k_t);
	control_relayx(relay_from_id(tier->relay_id));
}

void task_emergency_shutdown(void)
{
	ch224k_t.deception_vol = VOL_5V0;
	set_ch224k_deceptionVol(&ch224k_t);
	relay_close_all();
	app_state()->isOpen = DISABLE;
	protect_on_output_disabled();
}

static void clear_fault_latch(void)
{
	AppState_t *state = app_state();

	state->faultLatched = 0U;
	state->faultReason = FAULT_NONE;
	OLED_Clear();
	ui_mark_dirty();
	ui_render();
}

void task_page_next(void)
{
	AppState_t *state = app_state();

	state->currentPage++;
	if (state->currentPage > POWER_PAGE)
	{
		state->currentPage = VOL_PAGE;
	}
	ui_mark_dirty();
	ui_render();
}

void task_toggle_power_output(void)
{
	AppState_t *state = app_state();
	uint16_t vol_x100 = 0U;
	uint16_t cur_ma = 0U;

	if (state->faultLatched != 0U)
	{
		clear_fault_latch();
		return;
	}

	if (state->isOpen == DISABLE)
	{
		open_power_switch(state->setVolIndex);
		state->isOpen = ENABLE;
		state->currentPage = POWER_PAGE;
		protect_on_output_enabled();
		protect_reset_filter();
		if (protect_sample_now(&vol_x100, &cur_ma) != 0U)
		{
			ui_render();
		}
		else
		{
			ui_mark_dirty();
			ui_render();
		}
	}
	else
	{
		task_emergency_shutdown();
		protect_reset_filter();
		OLED_Clear();
		ui_mark_dirty();
		ui_render();
	}
}

void task_adjust_voltage_up(void)
{
	AppState_t *state = app_state();

	state->setVolIndex = power_profile_next_index(state->setVolIndex);
	ui_mark_dirty();
	ui_render();
}

void task_adjust_voltage_down(void)
{
	AppState_t *state = app_state();

	state->setVolIndex = power_profile_prev_index(state->setVolIndex);
	ui_mark_dirty();
	ui_render();
}

void task_adjust_current_up(void)
{
	AppState_t *state = app_state();

	if (state->protectValue <= (PROTECT_CUR_MAX_MA - PROTECT_CUR_STEP_MA))
	{
		state->protectValue += PROTECT_CUR_STEP_MA;
		ui_mark_dirty();
		ui_render();
	}
}

void task_adjust_current_down(void)
{
	AppState_t *state = app_state();

	if (state->protectValue > PROTECT_CUR_MIN_MA)
	{
		state->protectValue -= PROTECT_CUR_STEP_MA;
		ui_mark_dirty();
		ui_render();
	}
}

void init_task(void)
{
	app_state_reset_defaults();
	key_app_init();

	ch224k_t = ch224k_init();
	relay_t[0] = relay_init(CON_IO3_GPIO_Port, CON_IO3_Pin, GPIO_PIN_RESET);
	relay_t[1] = relay_init(CON_IO1_GPIO_Port, CON_IO1_Pin, GPIO_PIN_RESET);
	relay_t[2] = relay_init(CON_IO2_GPIO_Port, CON_IO2_Pin, GPIO_PIN_RESET);

	task_emergency_shutdown();

	OLED_Init();
	HAL_Delay(200);
	ui_init();
	ui_render();
	hw_adc_sample_init();
	MX_IWDG_Init();

	last_tick_ms = HAL_GetTick();
	last_display_ms = last_tick_ms;
}

void task_run(void)
{
	uint32_t now = HAL_GetTick();

	if ((now - last_tick_ms) >= TICK_MS)
	{
		last_tick_ms = now;
		key_app_poll();
		protect_tick();
		IWDG_UserRefresh();

		if ((now - last_display_ms) >= DISPLAY_PERIOD_MS)
		{
			last_display_ms = now;
			ui_display_tick();
		}
	}
}
