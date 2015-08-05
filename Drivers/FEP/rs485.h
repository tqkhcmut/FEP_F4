#ifndef _RS485_H_
#define _RS485_H_
#include <stm32f4xx.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// FE Protocol
#include "FEP/fep_define.h"
#include "FEP/fep_gpio.h"

/* Include header definitions here */

/* Redefine following parameters base on your desired */

/* Parameters */
// Parity
#ifndef RS485_PARITY
#define RS485_PARITY		 USART_Parity_No
#endif

// Stop bit
#ifndef RS485_STOP_BIT
#define RS485_STOP_BIT 		USART_StopBits_2
#endif

// Baudrate
// Default baudrate is 2500000
#ifndef RS485_BAUDRATE
#define RS485_BAUDRATE 		2500000
#endif


///////////// quan
#define UART_485					UART4
#define UART_485_IRQn			UART4_IRQn
#define UART_485_DIR_TRANSF		1
#define UART_485_DIR_RECE			0

#define UART_PORT_1				1
#define UART_PORT_2				2
#define UART_PORT_3				3
#define UART_PORT_4				4
#define UART_PORT_5				5
#define UART_PORT_6				6

/* End parameters */

typedef struct
{
	void 	(*Init)				(QueueHandle_t queue_buffer);
	
	// send FE packets
	// each FE packet end with break signal
	void 	(*SendPacket)		(FEP_Packet_t * packet, int NumOfPacket);
	void 	(*dir)(char dir_select);
	void  (*sel)(char port);
} RS485_API_t;

extern const RS485_API_t RS485_API;

#endif
