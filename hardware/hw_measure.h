#ifndef HW_MEASURE_H_
#define HW_MEASURE_H_

#include <stdint.h>

uint8_t hw_measure_from_raw(uint16_t raw_cur, uint16_t raw_vol, uint16_t raw_vref,
                            uint16_t *vol_x100, uint16_t *cur_ma);

#endif /* HW_MEASURE_H_ */
