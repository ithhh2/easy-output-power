#include "protect_app.h"

#include "app_config.h"
#include "hw_adc_sample.h"
#include "hw_measure.h"
#include "main.h"
#include "mid_oled.h"
#include "power_profile.h"

extern void ui_mark_dirty(void);

static uint16_t vol_filter[FILTER_LEN] = {0};
static uint16_t cur_filter[FILTER_LEN] = {0};
static uint32_t vol_filter_sum = 0U;
static uint32_t cur_filter_sum = 0U;
static uint8_t filter_idx = 0U;
static uint8_t filter_count = 0U;
static uint8_t fault_streak = 0U;
static uint8_t adc_fail_streak = 0U;
static uint32_t output_on_ms = 0U;
static uint32_t last_sample_ms = 0U;

static uint16_t filter_average_vol(void)
{
	if (filter_count == 0U)
	{
		return 0U;
	}

	return (uint16_t)(vol_filter_sum / filter_count);
}

static uint16_t filter_average_cur(void)
{
	if (filter_count == 0U)
	{
		return 0U;
	}

	return (uint16_t)(cur_filter_sum / filter_count);
}

static void push_filter(uint16_t vol_x100, uint16_t cur_ma)
{
	if (filter_count >= FILTER_LEN)
	{
		vol_filter_sum -= vol_filter[filter_idx];
		cur_filter_sum -= cur_filter[filter_idx];
	}
	else
	{
		filter_count++;
	}

	vol_filter[filter_idx] = vol_x100;
	cur_filter[filter_idx] = cur_ma;
	vol_filter_sum += vol_x100;
	cur_filter_sum += cur_ma;

	filter_idx++;
	if (filter_idx >= FILTER_LEN)
	{
		filter_idx = 0U;
	}
}

static uint8_t sample_measurements(uint16_t *vol_x100, uint16_t *cur_ma)
{
	uint16_t raw_cur = 0U;
	uint16_t raw_vol = 0U;
	uint16_t raw_vref = 0U;

	if (hw_adc_sample(&raw_cur, &raw_vol, &raw_vref) == 0U)
	{
		return 0U;
	}

	if (hw_measure_from_raw(raw_cur, raw_vol, raw_vref, vol_x100, cur_ma) == 0U)
	{
		return 0U;
	}

	return 1U;
}

void protect_reset_filter(void)
{
	filter_idx = 0U;
	filter_count = 0U;
	vol_filter_sum = 0U;
	cur_filter_sum = 0U;
	fault_streak = 0U;
	adc_fail_streak = 0U;
}

void protect_get_live(uint16_t *vol_x100, uint16_t *cur_ma)
{
	AppState_t *state = app_state();

	if ((vol_x100 == NULL) || (cur_ma == NULL))
	{
		return;
	}

	if (filter_count > 0U)
	{
		*vol_x100 = filter_average_vol();
		*cur_ma = filter_average_cur();
		state->volValue_x100 = *vol_x100;
		state->curValue = *cur_ma;
	}
	else
	{
		*vol_x100 = state->volValue_x100;
		*cur_ma = state->curValue;
	}
}

uint8_t protect_filter_count(void)
{
	return filter_count;
}

uint8_t protect_sample_now(uint16_t *vol_x100, uint16_t *cur_ma)
{
	AppState_t *state = app_state();

	if (sample_measurements(vol_x100, cur_ma) == 0U)
	{
		return 0U;
	}

	push_filter(*vol_x100, *cur_ma);
	state->volValue_x100 = *vol_x100;
	state->curValue = *cur_ma;
	adc_fail_streak = 0U;
	ui_mark_dirty();
	return 1U;
}

void protect_trigger_fault(FaultReason_t reason)
{
	extern void task_emergency_shutdown(void);

	task_emergency_shutdown();
	app_state()->faultLatched = 1U;
	app_state()->faultReason = reason;
	protect_reset_filter();
	OLED_Clear();
	ui_mark_dirty();
}

void protect_tick(void)
{
	uint16_t vol_x100 = 0U;
	uint16_t cur_ma = 0U;
	uint16_t vol_check_x100 = 0U;
	uint16_t cur_check_ma = 0U;
	AppState_t *state = app_state();
	const PowerTier_t *tier = power_profile_get(state->setVolIndex);
	uint16_t protect_min_x100 = tier->protect_min_x100;
	uint8_t fault_now = 0U;
	uint32_t now = HAL_GetTick();
	uint8_t in_grace = 0U;
	uint8_t sampled = 0U;

	if (state->isOpen != ENABLE)
	{
		return;
	}

	if ((now - output_on_ms) < PROTECT_GRACE_MS)
	{
		in_grace = 1U;
	}

	if ((now - last_sample_ms) >= PROTECT_SAMPLE_MS)
	{
		last_sample_ms = now;
		if (protect_sample_now(&vol_x100, &cur_ma) != 0U)
		{
			sampled = 1U;
		}
		else
		{
			adc_fail_streak++;
			if (adc_fail_streak >= ADC_FAIL_STREAK_LIMIT)
			{
				protect_trigger_fault(FAULT_ADC);
			}
		}
	}

	if (in_grace != 0U)
	{
		fault_streak = 0U;
		return;
	}

	if (filter_count >= PROTECT_UV_FILTER_MIN)
	{
		vol_check_x100 = filter_average_vol();
		cur_check_ma = filter_average_cur();
	}
	else if (sampled != 0U)
	{
		vol_check_x100 = vol_x100;
		cur_check_ma = cur_ma;
	}
	else
	{
		vol_check_x100 = state->volValue_x100;
		cur_check_ma = state->curValue;
	}

	if ((filter_count >= PROTECT_OC_FILTER_MIN) &&
	    (cur_check_ma > state->protectValue))
	{
		fault_now = 1U;
		if (fault_streak == 0U)
		{
			state->faultReason = FAULT_OVERCURRENT;
		}
	}
	else if ((filter_count >= PROTECT_UV_FILTER_MIN) &&
	         (vol_check_x100 < protect_min_x100))
	{
		fault_now = 1U;
		if (fault_streak == 0U)
		{
			state->faultReason = FAULT_UNDERVOLTAGE;
		}
	}

	if (fault_now != 0U)
	{
		fault_streak++;
		if (fault_streak >= FAULT_STREAK_LIMIT)
		{
			protect_trigger_fault(state->faultReason);
		}
	}
	else
	{
		fault_streak = 0U;
	}
}

void protect_on_output_enabled(void)
{
	output_on_ms = HAL_GetTick();
	last_sample_ms = 0U;
}

void protect_on_output_disabled(void)
{
	output_on_ms = 0U;
	last_sample_ms = 0U;
}
