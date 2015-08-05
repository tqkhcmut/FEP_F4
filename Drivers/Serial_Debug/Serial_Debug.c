/*----------------------------------------------------------------------------
* Name:    Serial.c
* Purpose: Low Level Serial Routines
* Note(s): possible defines select the used communication interface:
*            __DBG_ITM   - ITM SWO interface
*                        - USART1 interface  (default)
*----------------------------------------------------------------------------
* This file is part of the uVision/ARM development tools.
* This software may only be used under the terms of a valid, current,
* end user licence from KEIL for a compatible version of KEIL software
* development tools. Nothing else gives you the right to use this software.
*
* This software is supplied "AS IS" without warranties of any kind.
*
* Copyright (c) 2012 Keil - An ARM Company. All rights reserved.
*----------------------------------------------------------------------------*/

#include <stm32f4xx.h>                       /* STM32F10x definitions         */

#ifdef __DBG_ITM
volatile int ITM_RxBuffer = ITM_RXBUFFER_EMPTY;  /*  CMSIS Debug Input        */
#else
int Rx = -1, Tx = 1;
#endif

/*----------------------------------------------------------------------------
Initialize UART pins, Baudrate
*----------------------------------------------------------------------------*/
void SER_Init (int baudrate) 
{
#ifndef __DBG_ITM
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;	
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Periph clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
//	/* Enable the USART1 Pins Software Remapping */
//  	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);  
	
	//--------- Tx = A9---- Rx = A10 ----------
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Connect USART pins to AF */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);     
	
	USART_InitStructure.USART_BaudRate            = baudrate;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
 	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	USART_Cmd(USART1, ENABLE);
	
	/* Enable the USARTz Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 13;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

/*----------------------------------------------------------------------------
Write character to Serial Port
*----------------------------------------------------------------------------*/
int SER_PutChar (int c) {
	
#ifdef __DBG_ITM
	ITM_SendChar(c);
#else
	while(Tx > 0); // waiting for previous session completed
	Tx = c;
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	while (Tx  >= 0);
	return c;
#endif
}


/*----------------------------------------------------------------------------
Read character from Serial Port   (blocking read)
*----------------------------------------------------------------------------*/
int SER_GetChar (void) {
	
#ifdef __DBG_ITM
	while (ITM_CheckChar() != 1) __NOP();
	return (ITM_ReceiveChar());
#else
	while(Rx < 0); // waiting for previous session completed
	Rx = -1;
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	while (Rx < 0); // waiting for character come from USART
	return Rx;
#endif
}

void SerialDisAllINT(void)
{
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
}

/*----------------------------------------------------------------------------
Check if a character is received
*----------------------------------------------------------------------------*/
int SER_CheckChar (void) {
	
#ifdef __DBG_ITM
	return (ITM_CheckChar());
#else
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == USART_FLAG_RXNE)
	{
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
		return (1);
	}
	else
	{
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
		return (0);
	}
#endif
}


int getStr(char * str)
{
	int len = 0;
	while(1)
	{
		*str = SER_GetChar();
		if (*str == '\r' || *str == '\n' || *str == '\0')
		{
			*str = 0;
			break;
		}
		len++;
		str++;
	}
	return len;
}

int getBytes(char * _buffer, int _len)
{
	int len = 0;
	while(1)
	{
		if (len == _len)
		{
			*_buffer = 0;
			break;
		}
		*_buffer = SER_GetChar();
		
		/* uncomment these following lines to use 		*/
		/* this function like getString function 		*/
//		if (*_buffer == '\r' || *_buffer == '\n' || *_buffer == '\0')
//		{
//			*_buffer = 0;
//			break;
//		}
		len++;
		_buffer++;
	}
	return len;
}

void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		if (Rx < 0)
		{
			Rx = USART_ReceiveData(USART1);
			USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
		}
	}
	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{
		if (Tx >= 0)
		{
			USART_SendData(USART1, Tx);
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
			Tx = -1;
		}
	}
}
