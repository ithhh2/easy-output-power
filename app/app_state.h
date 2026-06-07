#ifndef APP_STATE_H_
#define APP_STATE_H_

#include <stdint.h>
#include "main.h"

#define VOL_PAGE    0x01U
#define CUR_PAGE    0x02U
#define POWER_PAGE  0x03U

typedef enum
{
	FAULT_NONE = 0,
	FAULT_OVERCURRENT,
	FAULT_UNDERVOLTAGE,
	FAULT_ADC
} FaultReason_t;

typedef struct
{
	uint8_t isOpen;
	uint8_t currentPage;
	uint8_t setVolIndex;
	uint16_t protectValue;
	uint16_t curValue;
	uint16_t volValue_x100;
	uint8_t faultLatched;
	FaultReason_t faultReason;
} AppState_t;

void app_state_reset_defaults(void);
AppState_t *app_state(void);

#endif /* APP_STATE_H_ */
