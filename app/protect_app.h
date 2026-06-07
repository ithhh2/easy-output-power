#ifndef PROTECT_APP_H_
#define PROTECT_APP_H_

#include <stdint.h>

#include "app_state.h"

void protect_reset_filter(void);
void protect_get_live(uint16_t *vol_x100, uint16_t *cur_ma);
uint8_t protect_filter_count(void);
uint8_t protect_sample_now(uint16_t *vol_x100, uint16_t *cur_ma);
void protect_tick(void);
void protect_trigger_fault(FaultReason_t reason);
void protect_on_output_enabled(void);
void protect_on_output_disabled(void);

#endif /* PROTECT_APP_H_ */
