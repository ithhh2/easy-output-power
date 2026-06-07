#include "power_profile.h"

const PowerTier_t power_tier_table[] = {
#if PWR_ENABLE_1V8
	{180, VOL_5V0, RELAY_ID_1V8, 80},
#endif
#if PWR_ENABLE_3V3
	{330, VOL_5V0, RELAY_ID_3V3, 180},
#endif
#if PWR_ENABLE_5V0
	{500, VOL_5V0, RELAY_ID_VBUS, 350},
#endif
#if PWR_ENABLE_9V0
	{900, VOL_9V0, RELAY_ID_VBUS, 700},
#endif
#if PWR_ENABLE_12V
	{1200, VOL_12V, RELAY_ID_VBUS, 950},
#endif
};

const uint8_t POWER_TIER_COUNT = (uint8_t)(sizeof(power_tier_table) / sizeof(power_tier_table[0]));

uint8_t power_profile_default_index(void)
{
	uint8_t i = 0U;

	for (i = 0U; i < POWER_TIER_COUNT; i++)
	{
		if (power_tier_table[i].display_x100 == PWR_DEFAULT_VOLTAGE_X100)
		{
			return i;
		}
	}

	return 0U;
}

const PowerTier_t *power_profile_get(uint8_t index)
{
	if (index >= POWER_TIER_COUNT)
	{
		return &power_tier_table[0];
	}

	return &power_tier_table[index];
}

uint8_t power_profile_next_index(uint8_t index)
{
	index++;
	if (index >= POWER_TIER_COUNT)
	{
		index = 0U;
	}

	return index;
}

uint8_t power_profile_prev_index(uint8_t index)
{
	if (index == 0U)
	{
		index = (uint8_t)(POWER_TIER_COUNT - 1U);
	}
	else
	{
		index--;
	}

	return index;
}
