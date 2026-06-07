#include "hw_adc_sample.h"

#include <stddef.h>

#include "adc.h"
#include "app_config.h"
#include "main.h"

#define ADC_CHANNEL_COUNT  3U

static uint16_t adc_dma_buf[ADC_CHANNEL_COUNT] = {0};
static volatile AdcSampleState_t adc_state = ADC_SAMPLE_IDLE;
static uint32_t adc_busy_start_ms = 0U;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
	{
		adc_state = ADC_SAMPLE_READY;
	}
}

static void adc_dma_recover(void)
{
	(void)HAL_ADC_Stop_DMA(&hadc);
	(void)HAL_ADC_Stop(&hadc);
	adc_state = ADC_SAMPLE_IDLE;
}

AdcSampleState_t hw_adc_sample_state(void)
{
	if (adc_state == ADC_SAMPLE_BUSY)
	{
		if ((HAL_GetTick() - adc_busy_start_ms) >= ADC_SAMPLE_TIMEOUT_MS)
		{
			adc_dma_recover();
			adc_state = ADC_SAMPLE_ERROR;
		}
	}

	return adc_state;
}

uint8_t hw_adc_sample_start(void)
{
	if (hw_adc_sample_state() == ADC_SAMPLE_BUSY)
	{
		return 0U;
	}

	if (hw_adc_sample_state() == ADC_SAMPLE_READY)
	{
		(void)HAL_ADC_Stop_DMA(&hadc);
	}

	adc_state = ADC_SAMPLE_BUSY;
	adc_busy_start_ms = HAL_GetTick();

	if (HAL_ADC_Start_DMA(&hadc, (uint32_t *)adc_dma_buf, ADC_CHANNEL_COUNT) != HAL_OK)
	{
		adc_dma_recover();
		adc_state = ADC_SAMPLE_ERROR;
		return 0U;
	}

	return 1U;
}

uint8_t hw_adc_sample_fetch(uint16_t *raw_cur, uint16_t *raw_vol, uint16_t *raw_vref)
{
	if ((raw_cur == NULL) || (raw_vol == NULL) || (raw_vref == NULL))
	{
		return 0U;
	}

	if (hw_adc_sample_state() != ADC_SAMPLE_READY)
	{
		return 0U;
	}

	*raw_cur = adc_dma_buf[0];
	*raw_vol = adc_dma_buf[1];
	*raw_vref = adc_dma_buf[2];

	(void)HAL_ADC_Stop_DMA(&hadc);
	adc_state = ADC_SAMPLE_IDLE;

	if (*raw_vref == 0U)
	{
		adc_state = ADC_SAMPLE_ERROR;
		return 0U;
	}

	return 1U;
}

void hw_adc_sample_abort(void)
{
	adc_dma_recover();
	adc_state = ADC_SAMPLE_IDLE;
}

uint8_t hw_adc_sample_sync(uint16_t *raw_cur, uint16_t *raw_vol, uint16_t *raw_vref)
{
	uint32_t start = HAL_GetTick();

	if (hw_adc_sample_start() == 0U)
	{
		return 0U;
	}

	while (hw_adc_sample_state() == ADC_SAMPLE_BUSY)
	{
		if ((HAL_GetTick() - start) >= ADC_SAMPLE_TIMEOUT_MS)
		{
			hw_adc_sample_abort();
			return 0U;
		}
		__WFI();
	}

	if (hw_adc_sample_state() != ADC_SAMPLE_READY)
	{
		return 0U;
	}

	return hw_adc_sample_fetch(raw_cur, raw_vol, raw_vref);
}

void hw_adc_sample_init(void)
{
	uint16_t dummy_cur = 0U;
	uint16_t dummy_vol = 0U;
	uint16_t dummy_vref = 0U;

	(void)HAL_ADCEx_Calibration_Start(&hadc);
	(void)hw_adc_sample_sync(&dummy_cur, &dummy_vol, &dummy_vref);
}
