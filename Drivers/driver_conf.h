#ifndef driver_conf_h_
#define driver_conf_h_

/* Free RTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
// 


// ETHENET *************
#define USE_ETHERNET 	1
#define RMII_MODE
#define USE_STM32F1x7xB_ELEK

// DHCP task priority
#define DHCP_TASK_PRIO   ( tskIDLE_PRIORITY + 2 )

// Comment following line to use static ip address
//#define USE_DHCP

/* IP address */
#define IP_ADDR0		192
#define IP_ADDR1		168
#define IP_ADDR2		1
#define IP_ADDR3		16

/* Subnet mask address */
#define NETMASK_ADDR0	255
#define NETMASK_ADDR1	255
#define NETMASK_ADDR2	255
#define NETMASK_ADDR3	0

/* Gateway address */
#define GW_ADDR0		192
#define GW_ADDR1		168
#define GW_ADDR2		1
#define GW_ADDR3		1

/* MAC ADDRESS*/
#define MAC_ADDR0   	20
#define MAC_ADDR1  		16
#define MAC_ADDR2  		12
#define MAC_ADDR3   	32
#define MAC_ADDR4   	32
#define MAC_ADDR5   	28
//~ETHENET *************

// USART *************
// to use usart (usart 1), you must make sure that these following
// conditions were validated:
// USART 1 have not used.
// two GPIO pin were not used: PB6, PB7 (base on ELEK STM32F107 board)

#define USE_USART 		1

// redefine your desired usart baudrate,
// if not, it have default value at 9600
#define USART_BAUDRATE	115200

//~USART *************


// buzzer
#define USE_BUZZER 1
#define BUZZER_NORMAL

// RTC timer
#define USE_RTC	0

// SST flash
#define USE_SST_FLASH 0

#endif
