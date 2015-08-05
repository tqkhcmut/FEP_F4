#include "buzzer.h"

#if USE_BUZZER

#define BUZZER_TASK_STACK_SIZE 	(configMINIMAL_STACK_SIZE)
#define BUZZER_TASK_PIORITY 	(tskIDLE_PRIORITY+2)

// PE3
#define BUZZER_PIN		GPIO_Pin_14
#define BUZZER_PORT		GPIOB
#define BUZZER_PERIPH	RCC_AHB1Periph_GPIOB	
#define BUZZER_ON		{GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);}
#define BUZZER_OFF		{GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);}

QueueHandle_t BuzzerQueue;
struct BuzzerData * BuzzerData;

void BuzzerServe(void * pvParameters);

void BuzzerInit(void)
{
#ifdef BUZZER_NORMAL
	// In normal mode, we toggle the GPIO connected to buzzer
	// pin control. PE3
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(BUZZER_PERIPH, ENABLE);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;
	
	GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);
	BUZZER_OFF;
#else
	// initial timer and config something
	// not implementation yet
#endif
	
	// initial queue
	BuzzerQueue = xQueueCreate(20, sizeof(struct BuzzerData));
	BuzzerData = pvPortMalloc(sizeof(struct BuzzerData));
	
	// create buzzer sevice
	xTaskCreate( BuzzerServe, "Buzzer Task", BUZZER_TASK_STACK_SIZE, NULL, 
				BUZZER_TASK_PIORITY, NULL );
}

// buzzer function task
void BuzzerServe(void * pvParameters)
{
	while(1)
	{
		if (xQueueReceive(BuzzerQueue, BuzzerData, portMAX_DELAY) == pdTRUE)
		{
#ifdef BUZZER_NORMAL
			if (BuzzerData->State)
			{
				BUZZER_ON;
			}
			else
			{
				BUZZER_OFF;
			}
			vTaskDelay(BuzzerData->Life);
#else
			
#endif
		}
	}
}


void BuzzerTick(int time_ms)
{
	struct BuzzerData data;
	data.State = 1;
	data.Life = time_ms;
	xQueueSend(BuzzerQueue, &data, 10);
	data.State = 0;
	data.Life = 0;
	xQueueSend(BuzzerQueue, &data, 10);
}
void BuzzerOn(void)
{
	struct BuzzerData data;
	data.State = 1;
	data.Life = 0;
	xQueueSend(BuzzerQueue, &data, 10);
}
void BuzzerOff(void)
{
	struct BuzzerData data;
	data.State = 0;
	data.Life = 0;
	xQueueSend(BuzzerQueue, &data, 10);
}

#endif

