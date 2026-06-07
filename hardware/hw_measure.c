#include "hw_measure.h"

#include <stddef.h>

uint8_t hw_measure_from_raw(uint16_t raw_cur, uint16_t raw_vol, uint16_t raw_vref,
                            uint16_t *vol_x100, uint16_t *cur_ma)
{
	float cur_a;
	float vol_v;

	if ((raw_vref == 0U) || (vol_x100 == NULL) || (cur_ma == NULL))
	{
		return 0U;
	}

	cur_a = (((float)raw_cur / (float)raw_vref) * HW_MEASURE_VREF_NOMINAL_V) /
	        HW_MEASURE_CUR_AMP_GAIN / HW_MEASURE_CUR_SHUNT_OHM;
	vol_v = (((float)raw_vol / (float)raw_vref) * HW_MEASURE_VREF_NOMINAL_V) *
	        HW_MEASURE_VOL_DIVIDER_NUM / HW_MEASURE_VOL_DIVIDER_DEN;

	if (cur_a < 0.0f)
	{
		cur_a = 0.0f;
	}
	if (vol_v < 0.0f)
	{
		vol_v = 0.0f;
	}

	*cur_ma = (uint16_t)(cur_a * 1000.0f + 0.5f);
	*vol_x100 = (uint16_t)(vol_v * 100.0f + 0.5f);
	return 1U;
}
