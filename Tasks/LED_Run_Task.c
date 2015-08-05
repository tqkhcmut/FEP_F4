#include "LED_Run_Task.h"

portTASK_FUNCTION_PROTO(LED_Run_Task, pvParameters);

#define LED_RUN_TASK_STACK			1 * configMINIMAL_STACK_SIZE
#define LED_RUN_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )


#define LED_Pin		GPIO_Pin_15
#define LED_Port		GPIOB
#define LED_Periph		RCC_AHB1Periph_GPIOB
#define LED_ON		{GPIO_SetBits(LED_Port, LED_Pin);}
#define LED_OFF	{GPIO_ResetBits(LED_Port, LED_Pin);}

uint8_t LED_State = 0;

void LED_Toggle(void)
{
	if (LED_State == 0)
	{
		LED_ON;
		LED_State = 1;
	}
	else
	{
		LED_OFF;
		LED_State = 0;
	}
}

void LED_Run_Task_Create(void)
{
	// initial GPIO
	GPIO_InitTypeDef GPIO_InitStructure;
	// enable peripherial
	RCC_AHB1PeriphClockCmd(LED_Periph, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = LED_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(LED_Port, &GPIO_InitStructure);
	
	// the signal was created in dali driver or gateway
	
	//
	xTaskCreate(LED_Run_Task, "DALI LEDs", 
				LED_RUN_TASK_STACK, NULL,
				LED_RUN_TASK_PRIORITY, NULL);	
}

portTASK_FUNCTION(LED_Run_Task, pvParameters)
{
	while(1)
	{
//		while (xSemaphoreTake(DALI_LEDs_Task_Signal, 10) == pdTRUE) // 10ms timeout
//		{
//			uint8_t i = 10;
//			while(i--)
//			{
//				DALI_LED_Toggle();
//				vTaskDelay(20);
//			}
//		}
		LED_Toggle();
		vTaskDelay(300);
	}
//	vTaskDelete(NULL);
}
