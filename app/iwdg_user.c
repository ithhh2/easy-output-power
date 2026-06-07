#include "iwdg_user.h"
#include "main.h"

static IWDG_HandleTypeDef hiwdg;

void MX_IWDG_Init(void)
{
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
	hiwdg.Init.Window = IWDG_WINDOW_DISABLE;
	hiwdg.Init.Reload = 625;
	(void)HAL_IWDG_Init(&hiwdg);
}

void IWDG_UserRefresh(void)
{
	(void)HAL_IWDG_Refresh(&hiwdg);
}

void HAL_IWDG_MspInit(IWDG_HandleTypeDef *hiwdg_handle)
{
	(void)hiwdg_handle;
}
