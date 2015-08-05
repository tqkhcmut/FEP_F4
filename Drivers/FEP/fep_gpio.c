#include "fep_gpio.h"
#include "fep_define.h"

void GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC |
						   RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);
	
	
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_LED_RUN_PIN;
	GPIO_Init(GPIO_LED_RUN, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_UART_DRIVER_PIN;
	GPIO_Init(GPIO_UART_DRIVER, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_BUZER_PIN;
	GPIO_Init(GPIO_BUZER, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_UART_SEL_1_PIN;
	GPIO_Init(GPIO_UART_SEL_1, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_UART_SEL_2_PIN;
	GPIO_Init(GPIO_UART_SEL_2, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_UART_SEL_3_PIN;
	GPIO_Init(GPIO_UART_SEL_3, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_UART_SEL_4_PIN;
	GPIO_Init(GPIO_UART_SEL_4, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_UART_SEL_5_PIN;
	GPIO_Init(GPIO_UART_SEL_5, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_UART_SEL_6_PIN;
	GPIO_Init(GPIO_UART_SEL_6, &GPIO_InitStructure);
	
}

void TIM3_interrup(uint16_t	ms)
{
	TIM_TimeBaseInitTypeDef  TIM3_TimeBaseStructure;
	NVIC_InitTypeDef 				 NVICTIM3_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	//cau hinh TIM3 tan so 10k, tuc la moi lan dem cua Period la 0,1ms
	//TIM3 ngat sau: inter_100us*0,1ms
	TIM3_TimeBaseStructure.TIM_Period = 10*ms;//delay 100ms
	TIM3_TimeBaseStructure.TIM_Prescaler = 16799;//(=72M/10k-1) tan so 10k, T=0,1ms
	TIM3_TimeBaseStructure.TIM_ClockDivision =0;
	TIM3_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM3_TimeBaseStructure);
	//cau hinh cho TIM3 ngat
	/* Enable the TIM3 global Interrupt */
	NVICTIM3_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVICTIM3_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVICTIM3_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVICTIM3_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVICTIM3_InitStructure);
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);//xoa co bao tran
	TIM_Cmd(TIM3,ENABLE);				  //khoi dong TIM3
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//
}

extern uint8_t Flag_GetData;
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Trigger);
		//khu vuc viet code
		Flag_GetData = ACTIVE;
		//ket thuc khu vuc viet code
		TIM_ClearFlag(TIM3,TIM_FLAG_Update);
		
//		LED_RUN_TOLGE;
	}
	
}

