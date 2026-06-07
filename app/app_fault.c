#include "app_fault.h"

static AppFaultHandler_t g_fault_handler = NULL;

void app_fault_register(AppFaultHandler_t handler)
{
	g_fault_handler = handler;
}

void app_fault_notify(FaultReason_t reason)
{
	if (g_fault_handler != NULL)
	{
		g_fault_handler(reason);
	}
}
