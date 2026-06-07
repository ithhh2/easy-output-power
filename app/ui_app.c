#include "ui_app.h"

#include "app_state.h"
#include "mid_oled.h"
#include "power_profile.h"
#include "protect_app.h"
#include "util_format.h"

static uint8_t ui_dirty = 1U;

static struct
{
	uint16_t vol_x100;
	uint16_t cur_ma;
	uint8_t currentPage;
	uint8_t isOpen;
	uint8_t faultLatched;
	FaultReason_t faultReason;
} ui_cache = {0};

static const PowerTier_t *ui_active_tier(void)
{
	return power_profile_get(app_state()->setVolIndex);
}

static void show_vol_row(uint8_t mode, uint16_t live_vol_x100)
{
	char data[8] = {0};
	AppState_t *state = app_state();
	const PowerTier_t *tier = ui_active_tier();

	if (state->isOpen == ENABLE)
	{
		OLED_ShowString(0, 1, (uint8_t *)"Now Vol:", 16, mode);
		format_voltage(data, live_vol_x100);
	}
	else
	{
		OLED_ShowString(0, 1, (uint8_t *)"Set Vol:", 16, mode);
		format_voltage(data, tier->display_x100);
	}

	OLED_ShowString(72, 1, (uint8_t *)data, 16, mode);
}

static void show_cur_row(uint8_t mode, uint16_t live_cur_ma)
{
	char data[8] = {0};
	AppState_t *state = app_state();

	if (state->isOpen == ENABLE)
	{
		OLED_ShowString(0, 21, (uint8_t *)"Now Cur:", 16, mode);
		format_current_ma(data, live_cur_ma);
	}
	else
	{
		OLED_ShowString(0, 21, (uint8_t *)"Set Cur:", 16, mode);
		format_current_ma(data, state->protectValue);
	}

	OLED_ShowString(72, 21, (uint8_t *)data, 16, mode);
}

static void show_power_line(uint8_t mode)
{
	AppState_t *state = app_state();

	if (state->faultLatched != 0U)
	{
		if (state->faultReason == FAULT_OVERCURRENT)
		{
			OLED_ShowString(28, 41, (uint8_t *)"FAULT OC ", 16, mode);
		}
		else if (state->faultReason == FAULT_UNDERVOLTAGE)
		{
			OLED_ShowString(28, 41, (uint8_t *)"FAULT UV ", 16, mode);
		}
		else
		{
			OLED_ShowString(28, 41, (uint8_t *)"FAULT ADC", 16, mode);
		}
	}
	else if (state->isOpen == DISABLE)
	{
		OLED_ShowString(28, 41, (uint8_t *)"Power OFF", 16, mode);
	}
	else
	{
		OLED_ShowString(28, 41, (uint8_t *)"Power ON ", 16, mode);
	}
}

static void ui_update_cache(AppState_t *state, uint16_t vol_x100, uint16_t cur_ma)
{
	ui_cache.vol_x100 = vol_x100;
	ui_cache.cur_ma = cur_ma;
	ui_cache.currentPage = state->currentPage;
	ui_cache.isOpen = state->isOpen;
	ui_cache.faultLatched = state->faultLatched;
	ui_cache.faultReason = state->faultReason;
}

static uint8_t ui_layout_changed(AppState_t *state)
{
	if (state->currentPage != ui_cache.currentPage)
	{
		return 1U;
	}
	if (state->isOpen != ui_cache.isOpen)
	{
		return 1U;
	}
	if (state->faultLatched != ui_cache.faultLatched)
	{
		return 1U;
	}
	if (state->faultReason != ui_cache.faultReason)
	{
		return 1U;
	}

	return 0U;
}

static void ui_render_values(uint16_t vol_x100, uint16_t cur_ma)
{
	char vol_data[8] = {0};
	char cur_data[8] = {0};
	AppState_t *state = app_state();
	const PowerTier_t *tier = ui_active_tier();

	if (state->isOpen == ENABLE)
	{
		format_voltage(vol_data, vol_x100);
		format_current_ma(cur_data, cur_ma);
	}
	else
	{
		format_voltage(vol_data, tier->display_x100);
		format_current_ma(cur_data, state->protectValue);
	}

	OLED_ShowString(72, 1, (uint8_t *)vol_data, 16, normal_display);
	OLED_ShowString(72, 21, (uint8_t *)cur_data, 16, normal_display);
	OLED_Refresh();
	ui_update_cache(state, vol_x100, cur_ma);
	ui_dirty = 0U;
}

void ui_init(void)
{
	ui_dirty = 1U;
}

void ui_mark_dirty(void)
{
	ui_dirty = 1U;
}

void ui_render(void)
{
	uint16_t vol_x100 = 0U;
	uint16_t cur_ma = 0U;
	uint8_t vol_mode = normal_display;
	uint8_t cur_mode = normal_display;
	uint8_t pwr_mode = normal_display;
	AppState_t *state = app_state();

	protect_get_live(&vol_x100, &cur_ma);

	if (state->isOpen == ENABLE)
	{
		vol_mode = normal_display;
		cur_mode = normal_display;
		pwr_mode = reverse_display;
	}
	else
	{
		if (state->currentPage == VOL_PAGE)
		{
			vol_mode = reverse_display;
		}
		if (state->currentPage == CUR_PAGE)
		{
			cur_mode = reverse_display;
		}
		if (state->currentPage == POWER_PAGE)
		{
			pwr_mode = reverse_display;
		}
	}

	OLED_DrawLine(0, 20, 128, 20, 1);
	OLED_DrawLine(0, 40, 128, 40, 1);
	OLED_DrawLine(65, 0, 65, 40, 1);

	show_vol_row(vol_mode, vol_x100);
	show_cur_row(cur_mode, cur_ma);
	show_power_line(pwr_mode);
	OLED_Refresh();
	ui_update_cache(state, vol_x100, cur_ma);
	ui_dirty = 0U;
}

void ui_display_tick(void)
{
	uint16_t vol_x100 = 0U;
	uint16_t cur_ma = 0U;
	AppState_t *state = app_state();

	if (ui_dirty == 0U)
	{
		return;
	}

	if (state->isOpen != ENABLE)
	{
		return;
	}

	if (protect_filter_count() == 0U)
	{
		return;
	}

	protect_get_live(&vol_x100, &cur_ma);

	if ((ui_layout_changed(state) == 0U) &&
	    ((vol_x100 != ui_cache.vol_x100) || (cur_ma != ui_cache.cur_ma)))
	{
		ui_render_values(vol_x100, cur_ma);
		return;
	}

	ui_render();
}
