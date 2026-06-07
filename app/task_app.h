#ifndef TASK_APP_H_
#define TASK_APP_H_

#include "main.h"
#include "hw_key.h"

#define VOL_PAGE    0x01
#define CUR_PAGE    0x02
#define POWER_PAGE  0x03

enum Key_e
{
	SET_KEY = 0,
	ADD_KEY,
	SUB_KEY,
	KEY_NUM
};

enum Relay_e
{
	Relay_1V8 = 0,
	Relay_Vbus,
	Relay_3V3,
	Relay_Num
};

typedef enum
{
	FAULT_NONE = 0,
	FAULT_OVERCURRENT,
	FAULT_UNDERVOLTAGE
} FaultReason_t;

struct SystemParam
{
	uint8_t isOpen;
	uint8_t currentPage;
	uint8_t setVolIndex;
	uint16_t protectValue;
	uint16_t curValue;
	uint16_t volValue_x100;
	uint8_t faultLatched;
	uint8_t faultReason;
};

extern KEY_HandleDef key[KEY_NUM];

void init_task(void);
void task_run(void);
void task_emergency_shutdown(void);

#endif