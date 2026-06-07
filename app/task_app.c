#include "task_app.h"
#include "adc.h"
#include "hw_adc_dma.h"
#include "hw_ch224.h"
#include "hw_con.h"
#include "hw_measure.h"
#include "iwdg_user.h"
#include "mid_oled.h"
#include "power_profile.h"
#include "util_format.h"

#define FILTER_LEN           16U
#define FAULT_STREAK_LIMIT   5U
#define PROTECT_UV_FILTER_MIN  4U
#define TICK_MS              10U
#define DISPLAY_PERIOD_MS    100U
#define PROTECT_GRACE_MS     1000U
#define PROTECT_SAMPLE_MS    50U
#define PROTECT_CUR_MIN_MA   200U
#define PROTECT_CUR_MAX_MA   1000U
#define PROTECT_CUR_STEP_MA  100U

static struct SystemParam systemparam = {0};
static CH224K_HandleDef ch224k_t = {0};
static Relay_HandleDef relay_t[Relay_Num] = {0};

static uint16_t vol_filter[FILTER_LEN] = {0};
static uint16_t cur_filter[FILTER_LEN] = {0};
static uint8_t filter_idx = 0U;
static uint8_t filter_count = 0U;
static uint8_t fault_streak = 0U;

static uint32_t last_tick_ms = 0U;
static uint32_t last_display_ms = 0U;
static uint32_t output_on_ms = 0U;
static uint32_t last_sample_ms = 0U;
static uint32_t key_repeat_anchor_ms[KEY_NUM] = {0};
static uint32_t key_repeat_last_ms[KEY_NUM] = {0};

KEY_HandleDef key[KEY_NUM] = {0};

static uint16_t filter_average(uint16_t *buffer);

static const PowerTier_t *get_active_tier(void)
{
	return power_profile_get(systemparam.setVolIndex);
}

static void show_vol_row(uint8_t mode, uint16_t live_vol_x100)
{
	char data[8] = {0};
	const PowerTier_t *tier = get_active_tier();

	if (systemparam.isOpen == ENABLE)
	{
		OLED_ShowString(0, 1, (uint8_t *)"Now Vol:", 16, mode);
		format_voltage(data, live_vol_x100);
		OLED_ShowString(72, 1, (uint8_t *)data, 16, mode);
		return;
	}

	OLED_ShowString(0, 1, (uint8_t *)"Set Vol:", 16, mode);
	power_profile_format_voltage(data, tier->display_x100);
	OLED_ShowString(72, 1, (uint8_t *)data, 16, mode);
}

static void show_cur_row(uint8_t mode, uint16_t live_cur_ma)
{
	char data[8] = {0};

	if (systemparam.isOpen == ENABLE)
	{
		OLED_ShowString(0, 21, (uint8_t *)"Now Cur:", 16, mode);
		format_current_ma(data, live_cur_ma);
		OLED_ShowString(72, 21, (uint8_t *)data, 16, mode);
		return;
	}

	OLED_ShowString(0, 21, (uint8_t *)"Set Cur:", 16, mode);
	format_current_ma(data, systemparam.protectValue);
	OLED_ShowString(72, 21, (uint8_t *)data, 16, mode);
}

static void show_power_line(uint8_t mode)
{
	if (systemparam.faultLatched != 0U)
	{
		if (systemparam.faultReason == (uint8_t)FAULT_OVERCURRENT)
		{
			OLED_ShowString(28, 41, (uint8_t *)"FAULT OC ", 16, mode);
		}
		else
		{
			OLED_ShowString(28, 41, (uint8_t *)"FAULT UV ", 16, mode);
		}
	}
	else if (systemparam.isOpen == DISABLE)
	{
		OLED_ShowString(28, 41, (uint8_t *)"Power OFF", 16, mode);
	}
	else
	{
		OLED_ShowString(28, 41, (uint8_t *)"Power ON ", 16, mode);
	}
}

static void get_live_measurements(uint16_t *vol_x100, uint16_t *cur_ma)
{
	if ((vol_x100 == NULL) || (cur_ma == NULL))
	{
		return;
	}

	if (filter_count > 0U)
	{
		*vol_x100 = filter_average(vol_filter);
		*cur_ma = filter_average(cur_filter);
		systemparam.volValue_x100 = *vol_x100;
		systemparam.curValue = *cur_ma;
	}
	else
	{
		*vol_x100 = systemparam.volValue_x100;
		*cur_ma = systemparam.curValue;
	}
}

static void render_main_ui(void)
{
	uint16_t vol_x100 = 0U;
	uint16_t cur_ma = 0U;
	uint8_t vol_mode = normal_display;
	uint8_t cur_mode = normal_display;
	uint8_t pwr_mode = normal_display;

	get_live_measurements(&vol_x100, &cur_ma);

	if (systemparam.isOpen == ENABLE)
	{
		vol_mode = normal_display;
		cur_mode = normal_display;
		pwr_mode = reverse_display;
	}
	else
	{
		if (systemparam.currentPage == VOL_PAGE)
		{
			vol_mode = reverse_display;
		}
		if (systemparam.currentPage == CUR_PAGE)
		{
			cur_mode = reverse_display;
		}
		if (systemparam.currentPage == POWER_PAGE)
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
}

static Relay_HandleDef *relay_from_id(uint8_t relay_id)
{
	switch (relay_id)
	{
	case RELAY_ID_1V8:
		return &relay_t[Relay_1V8];
	case RELAY_ID_3V3:
		return &relay_t[Relay_3V3];
	case RELAY_ID_VBUS:
	default:
		return &relay_t[Relay_Vbus];
	}
}

static void open_power_switch(uint8_t tier_index)
{
	const PowerTier_t *tier = power_profile_get(tier_index);

	if (systemparam.faultLatched != 0U)
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
	systemparam.isOpen = DISABLE;
	output_on_ms = 0U;
}

static void close_power_switch(void)
{
	task_emergency_shutdown();
}

static void reset_measure_filter(void)
{
	filter_idx = 0U;
	filter_count = 0U;
	fault_streak = 0U;
}

static void trigger_fault(FaultReason_t reason)
{
	close_power_switch();
	systemparam.faultLatched = 1U;
	systemparam.faultReason = (uint8_t)reason;
	reset_measure_filter();
	OLED_Clear();
	render_main_ui();
}

static uint8_t sample_measurements(uint16_t *vol_x100, uint16_t *cur_ma)
{
	uint16_t raw_cur = 0U;
	uint16_t raw_vol = 0U;
	uint16_t raw_vref = 0U;

	if (hw_adc_dma_sample(&raw_cur, &raw_vol, &raw_vref) == 0U)
	{
		return 0U;
	}

	if (hw_measure_from_raw(raw_cur, raw_vol, raw_vref, vol_x100, cur_ma) == 0U)
	{
		return 0U;
	}

	return 1U;
}

static uint16_t filter_average(uint16_t *buffer)
{
	uint32_t sum = 0U;
	uint8_t i = 0U;

	for (i = 0U; i < filter_count; i++)
	{
		sum += buffer[i];
	}

	if (filter_count == 0U)
	{
		return 0U;
	}

	return (uint16_t)(sum / filter_count);
}

static void push_filter(uint16_t vol_x100, uint16_t cur_ma)
{
	vol_filter[filter_idx] = vol_x100;
	cur_filter[filter_idx] = cur_ma;
	filter_idx++;
	if (filter_idx >= FILTER_LEN)
	{
		filter_idx = 0U;
	}
	if (filter_count < FILTER_LEN)
	{
		filter_count++;
	}
}

static void refresh_live_display(void)
{
	uint16_t vol_x100 = 0U;
	uint16_t cur_ma = 0U;

	if (sample_measurements(&vol_x100, &cur_ma) == 0U)
	{
		return;
	}

	push_filter(vol_x100, cur_ma);
	systemparam.volValue_x100 = vol_x100;
	systemparam.curValue = cur_ma;
	render_main_ui();
}

static void clear_fault_latch(void)
{
	systemparam.faultLatched = 0U;
	systemparam.faultReason = (uint8_t)FAULT_NONE;
	OLED_Clear();
	render_main_ui();
}

static void toggle_power_output(void)
{
	if (systemparam.faultLatched != 0U)
	{
		clear_fault_latch();
		return;
	}

	if (systemparam.isOpen == DISABLE)
	{
		open_power_switch(systemparam.setVolIndex);
		systemparam.isOpen = ENABLE;
		systemparam.currentPage = POWER_PAGE;
		output_on_ms = HAL_GetTick();
		last_sample_ms = 0U;
		reset_measure_filter();
		refresh_live_display();
	}
	else
	{
		close_power_switch();
		systemparam.isOpen = DISABLE;
		output_on_ms = 0U;
		reset_measure_filter();
		OLED_Clear();
		render_main_ui();
	}
}

static void adjust_voltage_up(void)
{
	systemparam.setVolIndex = power_profile_next_index(systemparam.setVolIndex);
	render_main_ui();
}

static void adjust_voltage_down(void)
{
	systemparam.setVolIndex = power_profile_prev_index(systemparam.setVolIndex);
	render_main_ui();
}

static void adjust_current_up(void)
{
	if (systemparam.protectValue <= (PROTECT_CUR_MAX_MA - PROTECT_CUR_STEP_MA))
	{
		systemparam.protectValue += PROTECT_CUR_STEP_MA;
		render_main_ui();
	}
}

static void adjust_current_down(void)
{
	if (systemparam.protectValue >= PROTECT_CUR_MIN_MA)
	{
		systemparam.protectValue -= PROTECT_CUR_STEP_MA;
		render_main_ui();
	}
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
	if ((systemparam.isOpen != DISABLE) ||
	    (systemparam.currentPage != VOL_PAGE && systemparam.currentPage != CUR_PAGE))
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
		if (systemparam.currentPage == VOL_PAGE)
		{
			adjust_voltage_up();
		}
		else
		{
			adjust_current_up();
		}
	}

	if (key_is_held(&key[SUB_KEY]) == 0U)
	{
		key_repeat_reset(SUB_KEY);
	}
	else if (key_repeat_due(SUB_KEY) != 0U)
	{
		if (systemparam.currentPage == VOL_PAGE)
		{
			adjust_voltage_down();
		}
		else
		{
			adjust_current_down();
		}
	}
}

static void key_handle(KEY_HandleDef *key_handle)
{
	uint8_t key_index = KEY_NUM;

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
		if (systemparam.isOpen == DISABLE)
		{
			systemparam.currentPage++;
			if (systemparam.currentPage > POWER_PAGE)
			{
				systemparam.currentPage = VOL_PAGE;
			}
			render_main_ui();
		}
		break;

	case SW2_Pin:
		switch (systemparam.currentPage)
		{
		case POWER_PAGE:
			toggle_power_output();
			break;
		case VOL_PAGE:
			if (systemparam.isOpen == DISABLE)
			{
				adjust_voltage_up();
				key_repeat_mark(ADD_KEY);
			}
			break;
		case CUR_PAGE:
			if (systemparam.isOpen == DISABLE)
			{
				adjust_current_up();
				key_repeat_mark(ADD_KEY);
			}
			break;
		default:
			break;
		}
		break;

	case SW3_Pin:
		switch (systemparam.currentPage)
		{
		case POWER_PAGE:
			toggle_power_output();
			break;
		case VOL_PAGE:
			if (systemparam.isOpen == DISABLE)
			{
				adjust_voltage_down();
				key_repeat_mark(SUB_KEY);
			}
			break;
		case CUR_PAGE:
			if (systemparam.isOpen == DISABLE)
			{
				adjust_current_down();
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
	    (systemparam.isOpen != DISABLE ||
	     (systemparam.currentPage != VOL_PAGE &&
	      systemparam.currentPage != CUR_PAGE)))
	{
		key_repeat_reset(key_index);
	}
}

static void key_poll(void)
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

static void protect_tick(void)
{
	uint16_t vol_x100 = 0U;
	uint16_t cur_ma = 0U;
	uint16_t vol_check_x100 = 0U;
	uint16_t cur_check_ma = 0U;
	const PowerTier_t *tier = get_active_tier();
	uint16_t protect_min_x100 = tier->protect_min_x100;
	uint8_t fault_now = 0U;
	uint32_t now = HAL_GetTick();
	uint8_t in_grace = 0U;
	uint8_t sampled = 0U;

	if (systemparam.isOpen != ENABLE)
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
		if (sample_measurements(&vol_x100, &cur_ma) != 0U)
		{
			push_filter(vol_x100, cur_ma);
			systemparam.volValue_x100 = vol_x100;
			systemparam.curValue = cur_ma;
			sampled = 1U;
			render_main_ui();
		}
	}

	if (in_grace != 0U)
	{
		fault_streak = 0U;
		return;
	}

	if (filter_count >= PROTECT_UV_FILTER_MIN)
	{
		vol_check_x100 = filter_average(vol_filter);
		cur_check_ma = filter_average(cur_filter);
	}
	else if (sampled != 0U)
	{
		vol_check_x100 = vol_x100;
		cur_check_ma = cur_ma;
	}
	else
	{
		vol_check_x100 = systemparam.volValue_x100;
		cur_check_ma = systemparam.curValue;
	}

	if (cur_check_ma > systemparam.protectValue)
	{
		fault_now = 1U;
		systemparam.faultReason = (uint8_t)FAULT_OVERCURRENT;
	}
	else if ((filter_count >= PROTECT_UV_FILTER_MIN) &&
	         (vol_check_x100 < protect_min_x100))
	{
		fault_now = 1U;
		systemparam.faultReason = (uint8_t)FAULT_UNDERVOLTAGE;
	}

	if (fault_now != 0U)
	{
		fault_streak++;
		if (fault_streak >= FAULT_STREAK_LIMIT)
		{
			trigger_fault((FaultReason_t)systemparam.faultReason);
		}
	}
	else
	{
		fault_streak = 0U;
	}
}

static void display_tick(void)
{
	if (systemparam.isOpen != ENABLE)
	{
		return;
	}

	if (filter_count == 0U)
	{
		return;
	}

	render_main_ui();
}

void init_task(void)
{
	systemparam.currentPage = POWER_PAGE;
	systemparam.isOpen = DISABLE;
	systemparam.setVolIndex = power_profile_default_index();
	systemparam.protectValue = 500U;
	systemparam.curValue = 0U;
	systemparam.volValue_x100 = 0U;
	systemparam.faultLatched = 0U;
	systemparam.faultReason = (uint8_t)FAULT_NONE;

	key[SET_KEY] = key_init(SW1_GPIO_Port, SW1_Pin, GPIO_PIN_RESET);
	key[ADD_KEY] = key_init(SW2_GPIO_Port, SW2_Pin, GPIO_PIN_RESET);
	key[SUB_KEY] = key_init(SW3_GPIO_Port, SW3_Pin, GPIO_PIN_RESET);

	ch224k_t = ch224k_init();
	relay_t[Relay_1V8] = relay_init(CON_IO3_GPIO_Port, CON_IO3_Pin, GPIO_PIN_RESET);
	relay_t[Relay_Vbus] = relay_init(CON_IO1_GPIO_Port, CON_IO1_Pin, GPIO_PIN_RESET);
	relay_t[Relay_3V3] = relay_init(CON_IO2_GPIO_Port, CON_IO2_Pin, GPIO_PIN_RESET);

	task_emergency_shutdown();

	OLED_Init();
	HAL_Delay(200);
	render_main_ui();
	(void)HAL_ADCEx_Calibration_Start(&hadc);
	hw_adc_dma_init();
	MX_IWDG_Init();

	last_tick_ms = HAL_GetTick();
	last_display_ms = last_tick_ms;
	last_sample_ms = last_tick_ms;
}

void task_run(void)
{
	uint32_t now = HAL_GetTick();

	if ((now - last_tick_ms) >= TICK_MS)
	{
		last_tick_ms = now;
		key_poll();
		protect_tick();
		IWDG_UserRefresh();

		if ((now - last_display_ms) >= DISPLAY_PERIOD_MS)
		{
			last_display_ms = now;
			display_tick();
		}
	}
}
