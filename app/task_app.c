/*
 * app.c
 *
 *  Created on: Nov 14, 2025
 *      Author: lceda
 */
#include "task_app.h"
#include "stdio.h"
#include "adc.h"
#include "hw_ch224.h"
#include "hw_con.h"
#include "mid_oled.h"

static struct SystemParam systemparam = {0};	//systemparam handler
static CH224K_HandleDef ch224k_t = {0};			//ch224k handler
static Relay_HandleDef relay_t[Relay_Num] = {0};//relay handler array
static uint8_t key_trigger_flag = 0;			//key reigger flag
static float curCumsumValue[100] = {0};			//current cumsum value
static float volCumsumValue[100] = {0};			//voltage cumsum value
static uint8_t cumsumNum = 0;					//cumsum Num

KEY_HandleDef key[KEY_NUM]={0};

static void defaultShowUI(void)
{
	char Data[16]={0};
	OLED_DrawLine(0,20,128,20,1);
	OLED_DrawLine(0,40,128,40,1);
	OLED_DrawLine(65,0,65,40,1);

	OLED_ShowString(0,1,(uint8_t *)"Set Vol:",16,1);
	switch(systemparam.setVolValue)
	{
		case VOL_1V8:
				sprintf(Data,"%2d.%1dV",1,8);
				OLED_ShowString(80,1,(uint8_t *)Data,16,1);
			break;
		case VOL_3V3:
				sprintf(Data,"%2d.%1dV",3,3);
				OLED_ShowString(80,1,(uint8_t *)Data,16,1);
			break;
		case VOL_5V0:
				sprintf(Data,"%2d.%1dV",5,0);
				OLED_ShowString(80,1,(uint8_t *)Data,16,1);
			break;
		case VOL_9V0:
				sprintf(Data,"%2d.%1dV",9,0);
				OLED_ShowString(80,1,(uint8_t *)Data,16,1);
			break;
		case VOL_12V:
				sprintf(Data,"%2d.%1dV",12,0);
				OLED_ShowString(80,1,(uint8_t *)Data,16,1);
			break;
		default:
				sprintf(Data,"%2d.%1dV",5,0);
				OLED_ShowString(80,1,(uint8_t *)Data,16,1);
			break;
	}

	OLED_ShowString(0,21,(uint8_t *)"Set Cur:",16,1);
	sprintf(Data,"%4dmA",systemparam.protectValue);
	OLED_ShowString(72,21,(uint8_t *)Data,16,1);

	OLED_ShowString(28,41,(uint8_t *)"Power OFF",16,0);
	OLED_Refresh();
}

static void setShowUI(uint8_t page, uint8_t mode,uint8_t isRefresh)
{
	char Data[16]={0};
	switch(page)
	{
		case POWER_PAGE:
			if(systemparam.isOpen == DISABLE){
				OLED_ShowString(28,41,(uint8_t *)"Power OFF",16,mode);
			}
			else{
				OLED_ShowString(28,41,(uint8_t *)"Power ON ",16,mode);
			}
			break;
		case VOL_PAGE:
			switch(systemparam.setVolValue)
			{
				case VOL_1V8:
						sprintf(Data,"%2d.%1dV",1,8);
						OLED_ShowString(80,1,(uint8_t *)Data,16,mode);
					break;
				case VOL_3V3:
						sprintf(Data,"%2d.%1dV",3,3);
						OLED_ShowString(80,1,(uint8_t *)Data,16,mode);
					break;
				case VOL_5V0:
						sprintf(Data,"%2d.%1dV",5,0);
						OLED_ShowString(80,1,(uint8_t *)Data,16,mode);
					break;
				case VOL_9V0:
						sprintf(Data,"%2d.%1dV",9,0);
						OLED_ShowString(80,1,(uint8_t *)Data,16,mode);
					break;
				case VOL_12V:
						sprintf(Data,"%2d.%1dV",12,0);
						OLED_ShowString(80,1,(uint8_t *)Data,16,mode);
					break;
				default:
						sprintf(Data,"%2d.%1dV",5,0);
						OLED_ShowString(80,1,(uint8_t *)Data,16,mode);
					break;
			}
			break;
		case CUR_PAGE:
				sprintf(Data,"%4dmA",systemparam.protectValue);
				OLED_ShowString(72,21,(uint8_t *)Data,16,mode);
			break;
	}
	if(isRefresh == ENABLE){
		OLED_Refresh();
	}
}

static void CurrentShowUI(float volValue,uint16_t curValue)
{
	char Data[16]={0};
	OLED_ShowString(0,1,(uint8_t *)"Now Vol:",16,1);
	sprintf(Data,"%2d.%1dV",(uint16_t)volValue,(uint16_t)(volValue * 10) % 10);
	OLED_ShowString(80,1,(uint8_t *)Data,16,1);

	OLED_ShowString(0,21,(uint8_t *)"Now Cur:",16,1);
	sprintf(Data,"%4dmA",curValue);
	OLED_ShowString(72,21,(uint8_t *)Data,16,1);

	OLED_ShowString(28,41,(uint8_t *)"Power ON ",16,0);
	OLED_Refresh();
}

static void OpenPowerSwitch(uint8_t value)
{
	switch(value)
	{
		case VOL_1V8:
		{
			ch224k_t.deception_vol = VOL_5V0;
			set_ch224k_deceptionVol(&ch224k_t);
			control_relayx(&relay_t[Relay_1V8]);
		}
		break;
		case VOL_3V3:
		{
			ch224k_t.deception_vol = VOL_5V0;
			set_ch224k_deceptionVol(&ch224k_t);
			control_relayx(&relay_t[Relay_3V3]);
		}
		break;
		case VOL_5V0:
		{
			ch224k_t.deception_vol = VOL_5V0;
			set_ch224k_deceptionVol(&ch224k_t);
			control_relayx(&relay_t[Relay_Vbus]);
		}
		break;
		case VOL_9V0:
		{
			ch224k_t.deception_vol = VOL_9V0;
			set_ch224k_deceptionVol(&ch224k_t);
			control_relayx(&relay_t[Relay_Vbus]);
		}
		break;
		case VOL_12V:
		{
			ch224k_t.deception_vol = VOL_12V;
			set_ch224k_deceptionVol(&ch224k_t);
			control_relayx(&relay_t[Relay_Vbus]);
		}
		break;
		default:
		{
			ch224k_t.deception_vol = VOL_5V0;
			set_ch224k_deceptionVol(&ch224k_t);
			control_relayx(&relay_t[Relay_Vbus]);
		}
		break;
	}
}

static void ClosePowerSwitch(void)
{
	ch224k_t.deception_vol = VOL_5V0;
	set_ch224k_deceptionVol(&ch224k_t);
	relay_close_all();
}

static void bubble_sort(float arr[], uint16_t len)
{
    uint16_t i = 0, j = 0;
    float temp = 0;
    for (i = 0; i < len - 1; i++)
    {
        for (j = 0; j < len - 1 - i; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void init_task(void)
{
	/* system parameter initialization */
	systemparam.currentPage = POWER_PAGE;	//default select power page
	systemparam.isOpen = DISABLE;			//default is close
	systemparam.setVolValue = VOL_5V0;		//default set output 5v
	systemparam.protectValue = 500;			//default protect 500mA
	systemparam.curValue = 0;				//default now current = 0
	systemparam.volValue = 0;				//default now voltage = 0

	key[SET_KEY] = key_init(SW1_GPIO_Port,SW1_Pin,GPIO_PIN_RESET);
	key[ADD_KEY] = key_init(SW2_GPIO_Port,SW2_Pin,GPIO_PIN_RESET);
	key[SUB_KEY] = key_init(SW3_GPIO_Port,SW3_Pin,GPIO_PIN_RESET);

	ch224k_t = ch224k_init();
	relay_t[Relay_1V8] = relay_init(CON_IO3_GPIO_Port,CON_IO3_Pin,GPIO_PIN_RESET);
	relay_t[Relay_Vbus] = relay_init(CON_IO1_GPIO_Port,CON_IO1_Pin,GPIO_PIN_RESET);
	relay_t[Relay_3V3] = relay_init(CON_IO2_GPIO_Port,CON_IO2_Pin,GPIO_PIN_RESET);

	OLED_Init();
	HAL_Delay(200);
	defaultShowUI();

	HAL_ADCEx_Calibration_Start(&hadc);

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	key_trigger_flag = 1;
}

/*
 * Function Content: when a key is pressed to interrupt,trigger scanf check
 * Function Parameter: null
 * Return Value: null
 */
void trigger_scanf(void)
{
	if(key_trigger_flag == 1)
	{
		for(int i=0;i<500;i++)
		{
			key_scanf(&key[SET_KEY]);
			key_scanf(&key[ADD_KEY]);
			key_scanf(&key[SUB_KEY]);
			if(key[SET_KEY].key_state != KEY_NoPress){
				key_handle(&key[SET_KEY]);
				break;
			}
			if(key[ADD_KEY].key_state != KEY_NoPress){
				key_handle(&key[ADD_KEY]);
				break;
			}
			if(key[SUB_KEY].key_state != KEY_NoPress){
				key_handle(&key[SUB_KEY]);
				break;
			}
			HAL_Delay(10);
		}
		key_trigger_flag = 0;
	}
}

void key_handle(KEY_HandleDef *key_handle)
{
	if(key_handle->key_state == KeyPress)
	{
		switch(key_handle->key_pin)
		{
		case SW1_Pin:
		{
			if(systemparam.isOpen == DISABLE)
			{
				setShowUI(systemparam.currentPage,normal_display,DISABLE);
				systemparam.currentPage++;
				if(systemparam.currentPage > POWER_PAGE)
				{
					systemparam.currentPage = VOL_PAGE;
				}
				setShowUI(systemparam.currentPage,reverse_display,ENABLE);
			}
		}
			break;
		case SW2_Pin:
		{
			switch(systemparam.currentPage)
			{
				case POWER_PAGE:
				{
					if(systemparam.isOpen == DISABLE){
						OpenPowerSwitch(systemparam.setVolValue);
						systemparam.isOpen = ENABLE;
						CurrentShowUI(systemparam.volValue,systemparam.curValue);
					}
					else
					{
						ClosePowerSwitch();
						systemparam.isOpen = DISABLE;
						OLED_Clear();
						defaultShowUI();
					}
				}
				break;
				case VOL_PAGE:
				{
					systemparam.setVolValue++;
					if(systemparam.setVolValue > VOL_12V){
						systemparam.setVolValue = VOL_1V8;
					}
					setShowUI(systemparam.currentPage,reverse_display,ENABLE);
				}
				break;
				case CUR_PAGE:
				{
					if(systemparam.protectValue <= 1400){
						systemparam.protectValue = systemparam.protectValue + 100;
						setShowUI(systemparam.currentPage,reverse_display,ENABLE);
					}
				}
				break;
			}
		}
			break;
		case SW3_Pin:
		{
			switch(systemparam.currentPage)
			{
				case POWER_PAGE:
				{
					if(systemparam.isOpen == DISABLE){
						OpenPowerSwitch(systemparam.setVolValue);
						systemparam.isOpen = ENABLE;
						CurrentShowUI(systemparam.volValue,systemparam.curValue);
					}
					else
					{
						ClosePowerSwitch();
						systemparam.isOpen = DISABLE;
						OLED_Clear();
						defaultShowUI();
					}
				}
				break;
				case VOL_PAGE:
				{
					systemparam.setVolValue--;
					if(systemparam.setVolValue < VOL_1V8)
					{
						systemparam.setVolValue = VOL_12V;
					}
					setShowUI(systemparam.currentPage,reverse_display,ENABLE);
				}
				break;
				case CUR_PAGE:
				{
					if(systemparam.protectValue >= 200){
						systemparam.protectValue = systemparam.protectValue - 100;
						setShowUI(systemparam.currentPage,reverse_display,ENABLE);
					}
				}
				break;
			}
		}
			break;
		default:
			break;
		}
	}
}

/*
 * Function Content: check scan voltage/current value,if value out of scope trigger protect
 * Function Parameter: null
 * Return Value: null
 */
void protectiveScan(void)
{
	uint8_t protectVolValue = 0;	//protect voltage value
	uint32_t tempValue[3] = {0};	//temporary array
	float tempVolValue = 0;			//temporary voltage
	float tempCurValue = 0;			//temporary current
	float volSumValue = 0, curSumValue = 0;
	if(systemparam.isOpen == ENABLE)
	{
		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc,100);
		tempValue[0] = HAL_ADC_GetValue(&hadc);

		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc,100);
		tempValue[1] = HAL_ADC_GetValue(&hadc);

		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc,100);
		tempValue[2] = HAL_ADC_GetValue(&hadc);

		tempCurValue = (tempValue[0] * 1.0f / tempValue[2]) * 1.24f;
		tempCurValue = (tempCurValue / 50.0f / 0.02f * 1000);

		tempVolValue = (tempValue[1] * 1.0f / tempValue[2]) * 1.24f;
		tempVolValue = (13.3f * tempVolValue)/3.3f;

		curCumsumValue[cumsumNum] = tempCurValue;
		volCumsumValue[cumsumNum] = tempVolValue;
		cumsumNum++;

		switch(systemparam.setVolValue)
		{
			case VOL_1V8:
				protectVolValue = 1;
				break;
			case VOL_3V3:
				protectVolValue = 2;
				break;
			case VOL_5V0:
				protectVolValue = 4;
				break;
			case VOL_9V0:
				protectVolValue = 8;
				break;
			case VOL_12V:
				protectVolValue = 11;
				break;
		}

		if((tempCurValue > systemparam.protectValue) || (tempVolValue < protectVolValue))
		{
			ClosePowerSwitch();
			systemparam.isOpen = DISABLE;
			defaultShowUI();
			cumsumNum = 0;
			volSumValue = 0;
			curSumValue = 0;
		}

		if(cumsumNum >= 99){
			bubble_sort(curCumsumValue,100);
			bubble_sort(volCumsumValue,100);
			for(int i=10;i<90;i++)
			{
				volSumValue = volSumValue + volCumsumValue[i];
				curSumValue = curSumValue + curCumsumValue[i];
			}
			tempVolValue = volSumValue / 80.0f;
			tempCurValue = (uint16_t)(curSumValue / 80.0f);
			CurrentShowUI(tempVolValue,tempCurValue);
			cumsumNum = 0;
			volSumValue = 0;
			curSumValue = 0;
		}
	}
}
