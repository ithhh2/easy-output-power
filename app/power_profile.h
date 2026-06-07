#ifndef POWER_PROFILE_H_
#define POWER_PROFILE_H_

#include <stdint.h>
#include "hw_ch224.h"

#define PWR_ENABLE_1V8   1
#define PWR_ENABLE_3V3   1
#define PWR_ENABLE_5V0   1
#define PWR_ENABLE_9V0   0
#define PWR_ENABLE_12V   0

#define PWR_DEFAULT_VOLTAGE_X100  500U  /* 上电默认 5.00V */

#if ((PWR_ENABLE_1V8 + PWR_ENABLE_3V3 + PWR_ENABLE_5V0 + PWR_ENABLE_9V0 + PWR_ENABLE_12V) == 0)
#error "At least one power tier must be enabled in power_profile.h"
#endif

#define RELAY_ID_1V8   0U
#define RELAY_ID_VBUS  1U
#define RELAY_ID_3V3   2U

typedef struct {
    uint16_t display_x100;
    uint8_t ch224_vol;
    uint8_t relay_id;
    uint16_t protect_min_x100;
} PowerTier_t;

extern const PowerTier_t power_tier_table[];
extern const uint8_t POWER_TIER_COUNT;

uint8_t power_profile_default_index(void);
const PowerTier_t *power_profile_get(uint8_t index);
uint8_t power_profile_next_index(uint8_t index);
uint8_t power_profile_prev_index(uint8_t index);
void power_profile_format_voltage(char *buf, uint16_t display_x100);

#endif
