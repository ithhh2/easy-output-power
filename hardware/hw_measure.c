#include "hw_measure.h"

#include <stddef.h>

#define MEASURE_VREF_NUM          1240U
#define MEASURE_VOL_NUM           164920U
#define MEASURE_VOL_DEN           330U
#define MEASURE_VOL_X100_MAX      2000U
#define MEASURE_CUR_MA_MAX        1500U

static uint16_t clamp_u16(uint32_t value, uint16_t max_value)
{
	if (value > (uint32_t)max_value)
	{
		return max_value;
	}

	return (uint16_t)value;
}

uint8_t hw_measure_from_raw(uint16_t raw_cur, uint16_t raw_vol, uint16_t raw_vref,
                            uint16_t *vol_x100, uint16_t *cur_ma)
{
	uint32_t cur_value;
	uint32_t vol_value;
	uint32_t vol_den;

	if ((raw_vref == 0U) || (vol_x100 == NULL) || (cur_ma == NULL))
	{
		return 0U;
	}

	cur_value = ((uint32_t)raw_cur * MEASURE_VREF_NUM + (raw_vref / 2U)) / raw_vref;
	*cur_ma = clamp_u16(cur_value, MEASURE_CUR_MA_MAX);

	vol_den = (uint32_t)raw_vref * MEASURE_VOL_DEN;
	vol_value = ((uint32_t)raw_vol * MEASURE_VOL_NUM + (vol_den / 2U)) / vol_den;
	*vol_x100 = clamp_u16(vol_value, MEASURE_VOL_X100_MAX);

	return 1U;
}
