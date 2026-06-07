#ifndef HW_ADC_DMA_H_
#define HW_ADC_DMA_H_

#include <stdint.h>

void hw_adc_dma_init(void);
uint8_t hw_adc_dma_sample(uint16_t *raw_cur, uint16_t *raw_vol, uint16_t *raw_vref);

#endif /* HW_ADC_DMA_H_ */
