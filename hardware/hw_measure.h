#ifndef HW_MEASURE_H_
#define HW_MEASURE_H_

#include <stdint.h>

#define HW_MEASURE_VREF_NOMINAL_V     1.24f
#define HW_MEASURE_ADC_RESOLUTION     4095.0f
#define HW_MEASURE_CUR_SHUNT_OHM      0.02f
#define HW_MEASURE_CUR_AMP_GAIN       50.0f
#define HW_MEASURE_VOL_DIVIDER_NUM    13.3f
#define HW_MEASURE_VOL_DIVIDER_DEN    3.3f

uint8_t hw_measure_from_raw(uint16_t raw_cur, uint16_t raw_vol, uint16_t raw_vref,
                            uint16_t *vol_x100, uint16_t *cur_ma);

#endif /* HW_MEASURE_H_ */
