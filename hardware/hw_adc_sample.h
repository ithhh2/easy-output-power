#ifndef HW_ADC_SAMPLE_H_
#define HW_ADC_SAMPLE_H_

#include <stdint.h>

typedef enum
{
	ADC_SAMPLE_IDLE = 0,
	ADC_SAMPLE_BUSY,
	ADC_SAMPLE_READY,
	ADC_SAMPLE_ERROR
} AdcSampleState_t;

void hw_adc_sample_init(void);
AdcSampleState_t hw_adc_sample_state(void);
uint8_t hw_adc_sample_start(void);
uint8_t hw_adc_sample_fetch(uint16_t *raw_cur, uint16_t *raw_vol, uint16_t *raw_vref);
void hw_adc_sample_abort(void);
uint8_t hw_adc_sample_sync(uint16_t *raw_cur, uint16_t *raw_vol, uint16_t *raw_vref);

#endif /* HW_ADC_SAMPLE_H_ */
