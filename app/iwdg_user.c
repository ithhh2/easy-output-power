#include "iwdg_user.h"
#include "main.h"

static IWDG_HandleTypeDef hiwdg;

void MX_IWDG_Init(void)
{
	/* LSI ~40kHz: timeout ~= Reload * Prescaler / LSI = 625 * 64 / 40000 ~= 1s */
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
	hiwdg.Init.Window = IWDG_WINDOW_DISABLE;
	hiwdg.Init.Reload = 625;
	if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
	{
		Error_Handler();
	}
}

void IWDG_UserRefresh(void)
{
	(void)HAL_IWDG_Refresh(&hiwdg);
}

void HAL_IWDG_MspInit(IWDG_HandleTypeDef *hiwdg_handle)
{
	(void)hiwdg_handle;
}
