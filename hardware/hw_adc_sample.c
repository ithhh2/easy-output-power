#include "hw_adc_sample.h"

#include <stddef.h>

#include "adc.h"
#include "main.h"

#define ADC_CHANNEL_COUNT      3U
#define ADC_DMA_TIMEOUT_MS     10U

static uint16_t adc_dma_buf[ADC_CHANNEL_COUNT] = {0};
static volatile uint8_t adc_dma_done = 0U;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
	{
		adc_dma_done = 1U;
	}
}

static uint8_t adc_dma_wait(uint32_t timeout_ms)
{
	uint32_t start = HAL_GetTick();

	while (adc_dma_done == 0U)
	{
		if ((HAL_GetTick() - start) >= timeout_ms)
		{
			return 0U;
		}
	}

	return 1U;
}

static void adc_dma_recover(void)
{
	(void)HAL_ADC_Stop_DMA(&hadc);
	(void)HAL_ADC_Stop(&hadc);
	adc_dma_done = 0U;
}

static uint8_t adc_dma_run_once(void)
{
	adc_dma_done = 0U;

	if (HAL_ADC_Start_DMA(&hadc, (uint32_t *)adc_dma_buf, ADC_CHANNEL_COUNT) != HAL_OK)
	{
		adc_dma_recover();
		return 0U;
	}

	if (adc_dma_wait(ADC_DMA_TIMEOUT_MS) == 0U)
	{
		adc_dma_recover();
		return 0U;
	}

	(void)HAL_ADC_Stop_DMA(&hadc);
	adc_dma_done = 0U;
	return 1U;
}

void hw_adc_sample_init(void)
{
	uint16_t dummy_cur = 0U;
	uint16_t dummy_vol = 0U;
	uint16_t dummy_vref = 0U;

	(void)HAL_ADCEx_Calibration_Start(&hadc);
	(void)hw_adc_sample(&dummy_cur, &dummy_vol, &dummy_vref);
}

uint8_t hw_adc_sample(uint16_t *raw_cur, uint16_t *raw_vol, uint16_t *raw_vref)
{
	if ((raw_cur == NULL) || (raw_vol == NULL) || (raw_vref == NULL))
	{
		return 0U;
	}

	if (adc_dma_run_once() == 0U)
	{
		return 0U;
	}

	*raw_cur = adc_dma_buf[0];
	*raw_vol = adc_dma_buf[1];
	*raw_vref = adc_dma_buf[2];

	if (*raw_vref == 0U)
	{
		return 0U;
	}

	return 1U;
}
