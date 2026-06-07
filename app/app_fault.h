#ifndef APP_FAULT_H_
#define APP_FAULT_H_

#include "app_state.h"

typedef void (*AppFaultHandler_t)(FaultReason_t reason);

void app_fault_register(AppFaultHandler_t handler);
void app_fault_notify(FaultReason_t reason);

#endif /* APP_FAULT_H_ */
