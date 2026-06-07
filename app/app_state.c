#include "app_state.h"
#include "power_profile.h"

static AppState_t g_state = {0};

void app_state_reset_defaults(void)
{
	g_state.currentPage = POWER_PAGE;
	g_state.isOpen = DISABLE;
	g_state.setVolIndex = power_profile_default_index();
	g_state.protectValue = 500U;
	g_state.curValue = 0U;
	g_state.volValue_x100 = 0U;
	g_state.faultLatched = 0U;
	g_state.faultReason = FAULT_NONE;
}

AppState_t *app_state(void)
{
	return &g_state;
}
