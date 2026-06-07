#include "hw_adc_dma.h"

#include <stddef.h>

#include "adc.h"

#define ADC_POLL_TIMEOUT_MS  100U

static uint8_t adc_poll_channel(uint16_t *raw)
{
	if (raw == NULL)
	{
		return 0U;
	}

	if (HAL_ADC_Start(&hadc) != HAL_OK)
	{
		return 0U;
	}

	if (HAL_ADC_PollForConversion(&hadc, ADC_POLL_TIMEOUT_MS) != HAL_OK)
	{
		return 0U;
	}

	*raw = (uint16_t)HAL_ADC_GetValue(&hadc);
	return 1U;
}

void hw_adc_dma_init(void)
{
	(void)HAL_ADC_Stop(&hadc);
}

uint8_t hw_adc_dma_sample(uint16_t *raw_cur, uint16_t *raw_vol, uint16_t *raw_vref)
{
	if ((raw_cur == NULL) || (raw_vol == NULL) || (raw_vref == NULL))
	{
		return 0U;
	}

	if (adc_poll_channel(raw_cur) == 0U)
	{
		(void)HAL_ADC_Stop(&hadc);
		return 0U;
	}
	if (adc_poll_channel(raw_vol) == 0U)
	{
		(void)HAL_ADC_Stop(&hadc);
		return 0U;
	}
	if (adc_poll_channel(raw_vref) == 0U)
	{
		(void)HAL_ADC_Stop(&hadc);
		return 0U;
	}

	if (*raw_vref == 0U)
	{
		(void)HAL_ADC_Stop(&hadc);
		return 0U;
	}

	(void)HAL_ADC_Stop(&hadc);
	return 1U;
}
