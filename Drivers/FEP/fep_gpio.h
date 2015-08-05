
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __fep_gpio_H
#define __fep_gpio_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
//#include "stm32f10x_gpio.h"
#include <stdio.h>
	 
// config led run and uart driver
#define GPIO_LED_RUN				GPIOB
#define GPIO_LED_RUN_PIN			GPIO_Pin_15
#define GPIO_UART_DRIVER			GPIOE
#define GPIO_UART_DRIVER_PIN		GPIO_Pin_5
#define GPIO_BUZER					GPIOB
#define GPIO_BUZER_PIN				GPIO_Pin_14
	 
#define GPIO_UART_SEL_1				GPIOC
#define GPIO_UART_SEL_1_PIN			GPIO_Pin_12
#define GPIO_UART_SEL_2				GPIOD
#define GPIO_UART_SEL_2_PIN			GPIO_Pin_1
#define GPIO_UART_SEL_3				GPIOD
#define GPIO_UART_SEL_3_PIN			GPIO_Pin_3
#define GPIO_UART_SEL_4				GPIOD
#define GPIO_UART_SEL_4_PIN			GPIO_Pin_5
#define GPIO_UART_SEL_5				GPIOB
#define GPIO_UART_SEL_5_PIN			GPIO_Pin_6
#define GPIO_UART_SEL_6				GPIOB
#define GPIO_UART_SEL_6_PIN			GPIO_Pin_9
	 
#define UART_SEL_1_HIGH				{GPIO_SetBits(GPIO_UART_SEL_1,GPIO_UART_SEL_1_PIN);}
#define UART_SEL_1_LOW				{GPIO_ResetBits(GPIO_UART_SEL_1,GPIO_UART_SEL_1_PIN);}
#define UART_SEL_2_HIGH				{GPIO_SetBits(GPIO_UART_SEL_2,GPIO_UART_SEL_2_PIN);}
#define UART_SEL_2_LOW				{GPIO_ResetBits(GPIO_UART_SEL_2,GPIO_UART_SEL_2_PIN);}
#define UART_SEL_3_HIGH				{GPIO_SetBits(GPIO_UART_SEL_3,GPIO_UART_SEL_3_PIN);}
#define UART_SEL_3_LOW				{GPIO_ResetBits(GPIO_UART_SEL_3,GPIO_UART_SEL_3_PIN);}
#define UART_SEL_4_HIGH				{GPIO_SetBits(GPIO_UART_SEL_4,GPIO_UART_SEL_4_PIN);}
#define UART_SEL_4_LOW				{GPIO_ResetBits(GPIO_UART_SEL_4,GPIO_UART_SEL_4_PIN);}
#define UART_SEL_5_HIGH				{GPIO_SetBits(GPIO_UART_SEL_5,GPIO_UART_SEL_5_PIN);}
#define UART_SEL_5_LOW				{GPIO_ResetBits(GPIO_UART_SEL_5,GPIO_UART_SEL_5_PIN);}
#define UART_SEL_6_HIGH				{GPIO_SetBits(GPIO_UART_SEL_6,GPIO_UART_SEL_6_PIN);}
#define UART_SEL_6_LOW				{GPIO_ResetBits(GPIO_UART_SEL_6,GPIO_UART_SEL_6_PIN);}

#define LED_RUN_ON					{GPIO_SetBits(GPIO_LED_RUN,GPIO_LED_RUN_PIN);}
#define LED_RUN_OFF					{GPIO_ResetBits(GPIO_LED_RUN,GPIO_LED_RUN_PIN);}
#define LED_RUN_TOLGE				{GPIO_WriteBit(GPIO_LED_RUN,GPIO_LED_RUN_PIN, 1-GPIO_ReadOutputDataBit(GPIO_LED_RUN,GPIO_LED_RUN_PIN));}
	 
#define UART_DRIVER_HIGH			{GPIO_SetBits(GPIO_UART_DRIVER,GPIO_UART_DRIVER_PIN);}
#define UART_DRIVER_LOW				{GPIO_ResetBits(GPIO_UART_DRIVER,GPIO_UART_DRIVER_PIN);}

#define BUZER_HIGH					{GPIO_SetBits(GPIO_BUZER,GPIO_BUZER_PIN);}
#define BUZER_LOW					{GPIO_ResetBits(GPIO_BUZER,GPIO_BUZER_PIN);}

void GPIO_Config(void);
void TIM3_interrup(uint16_t	ms);

#ifdef __cplusplus
}
#endif
  
#endif
