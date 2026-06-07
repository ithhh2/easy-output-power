#include "protect_app.h"

#include "app_config.h"
#include "app_fault.h"
#include "hw_adc_sample.h"
#include "hw_measure.h"
#include "power_profile.h"

extern void ui_mark_dirty(void);

static uint16_t vol_filter[FILTER_LEN] = {0};
static uint16_t cur_filter[FILTER_LEN] = {0};
static uint16_t vol_median_buf[MEDIAN_FILTER_LEN] = {0};
static uint16_t cur_median_buf[MEDIAN_FILTER_LEN] = {0};
static uint32_t vol_filter_sum = 0U;
static uint32_t cur_filter_sum = 0U;
static uint8_t filter_idx = 0U;
static uint8_t filter_count = 0U;
static uint8_t median_idx = 0U;
static uint8_t median_count = 0U;
static uint8_t fault_streak = 0U;
static uint8_t adc_fail_streak = 0U;
static uint8_t sample_pending = 0U;
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

static uint16_t median_of_buffer(uint16_t *buffer, uint8_t count)
{
	uint16_t tmp[MEDIAN_FILTER_LEN] = {0};
	uint8_t i = 0U;
	uint8_t j = 0U;
	uint16_t swap = 0U;

	if (count == 0U)
	{
		return 0U;
	}

	for (i = 0U; i < count; i++)
	{
		tmp[i] = buffer[i];
	}

	for (i = 0U; i < count; i++)
	{
		for (j = (uint8_t)(i + 1U); j < count; j++)
		{
			if (tmp[j] < tmp[i])
			{
				swap = tmp[i];
				tmp[i] = tmp[j];
				tmp[j] = swap;
			}
		}
	}

	return tmp[count / 2U];
}

static void median_push(uint16_t vol_x100, uint16_t cur_ma,
                        uint16_t *out_vol_x100, uint16_t *out_cur_ma)
{
	vol_median_buf[median_idx] = vol_x100;
	cur_median_buf[median_idx] = cur_ma;
	median_idx++;
	if (median_idx >= MEDIAN_FILTER_LEN)
	{
		median_idx = 0U;
	}
	if (median_count < MEDIAN_FILTER_LEN)
	{
		median_count++;
	}

	*out_vol_x100 = median_of_buffer(vol_median_buf, median_count);
	*out_cur_ma = median_of_buffer(cur_median_buf, median_count);
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

static uint8_t process_measurement(uint16_t raw_cur, uint16_t raw_vol, uint16_t raw_vref,
                                    uint16_t *vol_x100, uint16_t *cur_ma)
{
	uint16_t measured_vol = 0U;
	uint16_t measured_cur = 0U;
	uint16_t filtered_vol = 0U;
	uint16_t filtered_cur = 0U;
	AppState_t *state = app_state();

	if (hw_measure_from_raw(raw_cur, raw_vol, raw_vref, &measured_vol, &measured_cur) == 0U)
	{
		return 0U;
	}

	median_push(measured_vol, measured_cur, &filtered_vol, &filtered_cur);
	push_filter(filtered_vol, filtered_cur);
	*vol_x100 = filter_average_vol();
	*cur_ma = filter_average_cur();
	state->volValue_x100 = *vol_x100;
	state->curValue = *cur_ma;
	adc_fail_streak = 0U;
	ui_mark_dirty();
	return 1U;
}

static void get_check_values(uint16_t *vol_check_x100, uint16_t *cur_check_ma)
{
	AppState_t *state = app_state();

	if (filter_count >= PROTECT_UV_FILTER_MIN)
	{
		*vol_check_x100 = filter_average_vol();
		*cur_check_ma = filter_average_cur();
	}
	else
	{
		*vol_check_x100 = state->volValue_x100;
		*cur_check_ma = state->curValue;
	}
}

static void evaluate_faults(uint8_t allow_uv)
{
	AppState_t *state = app_state();
	const PowerTier_t *tier = power_profile_get(state->setVolIndex);
	uint16_t vol_check_x100 = 0U;
	uint16_t cur_check_ma = 0U;
	uint16_t oc_threshold = 0U;
	uint8_t fault_now = 0U;

	get_check_values(&vol_check_x100, &cur_check_ma);
	oc_threshold = (uint16_t)(state->protectValue + PROTECT_OC_HYST_MA);

	if ((filter_count >= PROTECT_OC_FILTER_MIN) && (cur_check_ma > oc_threshold))
	{
		fault_now = 1U;
		if (fault_streak == 0U)
		{
			state->faultReason = FAULT_OVERCURRENT;
		}
	}
	else if ((allow_uv != 0U) &&
	         (filter_count >= PROTECT_UV_FILTER_MIN) &&
	         (vol_check_x100 < tier->protect_min_x100))
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

static void handle_adc_failure(void)
{
	adc_fail_streak++;
	if (adc_fail_streak >= ADC_FAIL_STREAK_LIMIT)
	{
		protect_trigger_fault(FAULT_ADC);
	}
}

void protect_reset_filter(void)
{
	filter_idx = 0U;
	filter_count = 0U;
	median_idx = 0U;
	median_count = 0U;
	vol_filter_sum = 0U;
	cur_filter_sum = 0U;
	fault_streak = 0U;
	adc_fail_streak = 0U;
	sample_pending = 0U;
	hw_adc_sample_abort();
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
	uint16_t raw_cur = 0U;
	uint16_t raw_vol = 0U;
	uint16_t raw_vref = 0U;

	if (hw_adc_sample_sync(&raw_cur, &raw_vol, &raw_vref) == 0U)
	{
		return 0U;
	}

	return process_measurement(raw_cur, raw_vol, raw_vref, vol_x100, cur_ma);
}

void protect_trigger_fault(FaultReason_t reason)
{
	app_fault_notify(reason);
}

void protect_tick(void)
{
	uint16_t raw_cur = 0U;
	uint16_t raw_vol = 0U;
	uint16_t raw_vref = 0U;
	uint16_t vol_x100 = 0U;
	uint16_t cur_ma = 0U;
	AppState_t *state = app_state();
	uint32_t now = HAL_GetTick();
	uint8_t in_grace = 0U;
	AdcSampleState_t adc_state = ADC_SAMPLE_IDLE;

	if (state->isOpen != ENABLE)
	{
		return;
	}

	if ((now - output_on_ms) < PROTECT_GRACE_MS)
	{
		in_grace = 1U;
	}

	if (sample_pending != 0U)
	{
		adc_state = hw_adc_sample_state();
		if (adc_state == ADC_SAMPLE_READY)
		{
			if (hw_adc_sample_fetch(&raw_cur, &raw_vol, &raw_vref) != 0U)
			{
				(void)process_measurement(raw_cur, raw_vol, raw_vref, &vol_x100, &cur_ma);
			}
			else
			{
				handle_adc_failure();
			}
			sample_pending = 0U;
		}
		else if (adc_state == ADC_SAMPLE_ERROR)
		{
			handle_adc_failure();
			sample_pending = 0U;
		}
	}

	if ((sample_pending == 0U) && ((now - last_sample_ms) >= PROTECT_SAMPLE_MS))
	{
		last_sample_ms = now;
		if (hw_adc_sample_start() != 0U)
		{
			sample_pending = 1U;
		}
		else
		{
			handle_adc_failure();
		}
	}

	if (in_grace != 0U)
	{
		evaluate_faults(0U);
		return;
	}

	evaluate_faults(1U);
}

void protect_on_output_enabled(void)
{
	output_on_ms = HAL_GetTick();
	last_sample_ms = 0U;
	sample_pending = 0U;
}

void protect_on_output_disabled(void)
{
	output_on_ms = 0U;
	last_sample_ms = 0U;
	sample_pending = 0U;
	hw_adc_sample_abort();
}
