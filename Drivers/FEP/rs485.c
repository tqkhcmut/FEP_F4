#include "rs485.h"
#include "stdio.h"

// This procedure initialize Serial port and RS485 interface
static void RS485_Init(QueueHandle_t queue_buffer);

// This function sending data in buf with length is len to RS485 bus
static void RS485_SendPacket(FEP_Packet_t * packet, int NumOfPacket);
static void RS485_Dir(char dir_select);
static void RS485_sel(char port);

static QueueHandle_t __queue_buffer;
uint8_t data_buff[10];
static int rx_index;

const RS485_API_t RS485_API = 
{
	.Init 			= RS485_Init,
	.SendPacket = RS485_SendPacket,
	.dir				= RS485_Dir,
	.sel				= RS485_sel
};

static void  RS485_sel(char port)
{
	UART_SEL_1_HIGH;
	UART_SEL_2_HIGH;
	UART_SEL_3_HIGH;
	UART_SEL_4_HIGH;
	UART_SEL_5_HIGH;
	UART_SEL_6_HIGH;
	switch(port)
	{
		case UART_PORT_1:
		{
			UART_SEL_1_LOW;
			break;
		}
		case UART_PORT_2:
		{
			UART_SEL_2_LOW;
			break;
		}
		case UART_PORT_3:
		{
			UART_SEL_3_LOW;
			break;
		}
		case UART_PORT_4:
		{
			UART_SEL_4_LOW;
			break;
		}
		case UART_PORT_5:
		{
			UART_SEL_5_LOW;
			break;
		}
		case UART_PORT_6:
		{
			UART_SEL_6_LOW;
			break;
		}
		default:
			break;
	}
}
static void RS485_Dir(char dir_select)
{
	if(dir_select == UART_485_DIR_TRANSF)
	{
		//STM_EVAL_LEDOn(LED2);
		UART_DRIVER_HIGH;
	}
	else
	{
		//STM_EVAL_LEDOff(LED2);
		UART_DRIVER_LOW;
	}
}
// This procedure initialize Serial port and RS485 interface
void RS485_Init(QueueHandle_t queue_buffer)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;	
	NVIC_InitTypeDef NVIC_InitStructure;
		
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/* Connect USART pins to AF */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4); 
	
	USART_InitStructure.USART_BaudRate = 250000;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

	USART_Init(UART_485, &USART_InitStructure);
	
	USART_ITConfig(UART_485, USART_IT_RXNE, ENABLE);
 	USART_ITConfig(UART_485, USART_IT_FE, ENABLE);
	USART_Cmd(UART_485, ENABLE);
	
	/* Enable the USARTz Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART_485_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 13;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	__queue_buffer = queue_buffer;
}

// This function sending data in buf with length is len to RS485 bus
void RS485_SendPacket(FEP_Packet_t * packet, int NumOfPacket)
{
	FEP_Packet_t * tmpPacket = packet;
	int i = 0;
//	int j=750;
	RS485_API.dir(UART_485_DIR_TRANSF);
	for (i = 0; i < NumOfPacket && tmpPacket != NULL; i++)
	{
		USART_SendData(UART_485, tmpPacket->start_state);				// 1
		/* Loop until the end of transmission */
		while (USART_GetFlagStatus(UART_485, USART_FLAG_TC) == RESET)
		{}
		
		USART_SendData(UART_485, tmpPacket->size);					// 2
		/* Loop until the end of transmission */
		while (USART_GetFlagStatus(UART_485, USART_FLAG_TC) == RESET)
		{}
		
		USART_SendData(UART_485, tmpPacket->dev_info);				// 3
		/* Loop until the end of transmission */
		while (USART_GetFlagStatus(UART_485, USART_FLAG_TC) == RESET)
		{}
		
		USART_SendData(UART_485, tmpPacket->value);					// 4
		/* Loop until the end of transmission */
		while (USART_GetFlagStatus(UART_485, USART_FLAG_TC) == RESET)
		{}
		
		USART_SendData(UART_485, tmpPacket->checksum);				// 5
		/* Loop until the end of transmission */
		while (USART_GetFlagStatus(UART_485, USART_FLAG_TC) == RESET)
		{}
		
		USART_SendBreak(UART_485);									// break
		/* Loop until the end of transmission */
		while (USART_GetFlagStatus(UART_485, USART_FLAG_TC) == RESET)
		{}
//		while(j--);
		// delay 2ms base on FE protocol
		// delay occur on function call this, so do not call in interrupt or idle task
		vTaskDelay(2);
		
		tmpPacket = tmpPacket->next;
	}
	RS485_API.dir(UART_485_DIR_RECE);
//	vTaskDelay(1);
}

void UART4_IRQHandler(void)
{
	int i;
	if(USART_GetITStatus(UART_485, USART_IT_FE) != RESET)
	{
		for(i=0;i<6;i++)
		{
			data_buff[i]=data_buff[i+1];
		}
		FEP_Packet_t * tmpPacket = (FEP_Packet_t *)(data_buff);
//		if (PACKET_VALID(tmpPacket))
		{
			if (__queue_buffer != NULL)
				xQueueSendFromISR(__queue_buffer, tmpPacket, NULL);
		}
		//LED_RUN_TOLGE;
		//printf("%x,%x,%x,%x,%x,%x\n", data_buff[0],data_buff[1],data_buff[2],data_buff[3],data_buff[4],data_buff[5]);
		rx_index = 0;
	}
	if(USART_GetITStatus(UART_485, USART_IT_RXNE) != RESET)
	{
		data_buff[rx_index] = USART_ReceiveData(UART_485);
		rx_index++;
		if (rx_index > 10)
		{
			rx_index = 0;
		}
	}
	
}

