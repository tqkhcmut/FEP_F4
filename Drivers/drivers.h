#ifndef drivers_h_
#define drivers_h_

// all driver config must be in "driver_conf.h"
#include "driver_conf.h"

#ifndef byte
typedef unsigned char byte;
#endif


// Ethernet *****************************************
#if (USE_ETHERNET)      // use Ethernet

#include "Ethernet/netconf.h"
#include "Ethernet/stm32_eth.h"
#include "Ethernet/stm32_eth_bsp.h"
#include "lwip/tcpip.h"

void printd(const char * str, ...);

/* Ethernet data structure */
struct EthernetData
{
	struct ip_addr _ip;
	struct ip_addr _subnet;
	struct ip_addr _gateway;
};
#endif
// ~Ethernet *****************************************

// USART *****************************************
#if USE_USART

#ifndef USART_BAUDRATE
#define USART_BAUDRATE 9600 // default baudrate
#endif	// USART_BAUDRATE

extern SemaphoreHandle_t Serial_Sem;

// most of USART functionality is retarget in printf function
// the scanf function has some error. 
// Some useful USART functions were listed bellow.
// see more at Serial.c

// Get a character (blocking)
extern int SER_GetChar (void);
// Send a character (non-blocking)
extern int SER_PutChar (int c);
// Get a string (blocking)
extern int getStr(char * str);
// Get a serial desired bytes (blocking)
extern int getBytes(char * _buffer, int _len);

#endif	// USE_USART
// ~USART *****************************************

void DriversInit(void);

// relay
#ifndef USE_RELAY
#define USE_RELAY 0
#endif
#if USE_RELAY
#include "Relays/relays.h"
#endif

// input 220v
#ifndef USE_INPUT
#define USE_INPUT 0
#endif
#if USE_INPUT
#include "Input/input.h"
#endif

#ifndef USE_BUZZER
#define USE_BUZZER 0
#endif

#if USE_BUZZER
#include "Buzzer/buzzer.h"
#endif

// sim900
#ifndef USE_SIM900
#define USE_SIM900 0
#endif
#if USE_SIM900
#include "Sim900/sim900.h"
#endif

// RTC timer
#ifndef USE_RTC
#define USE_RTC 0
#endif
#if USE_RTC
#include "Time/hal_rtc.h"
#endif

#ifndef USE_BUTTONS
#define USE_BUTTONS 0
#endif
#if USE_BUTTONS
#include "Buttons/hal_buttons.h"
#endif

// SST flash
#ifndef USE_SST_FLASH
#define USE_SST_FLASH 0
#endif
#if USE_SST_FLASH
// external flash
#include "SST_Flash/spi_flash.h"

// Firmware data space: 				0x000000 -> 0x0FFFFF (1MB)
// DALI data space: 					0x100000 -> 0x100FFF (4KB)
// 4KB for Web data 					0x101000 -> 0x101FFF (4KB)
// 4KB for ethernet data (IP addr ...) 	0x102000 -> 0x102FFF (4K)
// something else: higher than 0x103000 (~900KB space)
#define FIRMWARE_SAVED_ADDR		0x000000UL  // 1MB
#define DALI_DATA_SAVED_ADDR	0x100000UL  // 4K
#define WEB_DATA_SAVED_ADDR		0x101000UL  // 4K
#define ETHERNET_DATA_SAVE_ADDR	0x102000UL	// 4K

// use for sync data
typedef enum
{
	SAVE = 's',
	RESTORE = 'r'
} SYNC_CMD_t;

#endif

#endif
