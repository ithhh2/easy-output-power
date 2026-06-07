#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#include <stdint.h>

#define FILTER_LEN              16U
#define MEDIAN_FILTER_LEN       5U
#define FAULT_STREAK_LIMIT      5U
#define ADC_FAIL_STREAK_LIMIT   5U
#define PROTECT_UV_FILTER_MIN   4U
#define PROTECT_OC_FILTER_MIN   4U
#define PROTECT_OC_HYST_MA      50U
#define TICK_MS                 10U
#define DISPLAY_PERIOD_MS       100U
#define PROTECT_GRACE_MS        1000U
#define PROTECT_SAMPLE_MS       50U
#define PROTECT_CUR_MIN_MA      200U
#define PROTECT_CUR_MAX_MA      1000U
#define PROTECT_CUR_STEP_MA     100U
#define STORAGE_SAVE_DEBOUNCE_MS 2000U
#define ADC_SAMPLE_TIMEOUT_MS   10U

#endif /* APP_CONFIG_H_ */
