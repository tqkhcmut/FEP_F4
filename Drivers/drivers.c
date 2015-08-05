#include "drivers.h"

#include <stdio.h>
#include "ethernetif.h"
#include "FEP/fep_gpio.h"
#include "FEP/rs485.h"

#if USE_USART
extern void SER_Init (int baudrate) ;
SemaphoreHandle_t Serial_Sem;
#endif

#if USE_ETHERNET
extern struct netif xnetif;
struct EthernetData ethData;

// for IP address
void ChangeStaticIP(void);

#endif

SemaphoreHandle_t fep_dev_list_Sem; //semiphore bao ve list fep_dev_list

void printd(const char * str, ...)
{
	if (xSemaphoreTake(Serial_Sem, 1) == pdTRUE) // 1ms timeout
	{
		printf(str);
		xSemaphoreGive(Serial_Sem);
	}
}

void DriversInit(void)
{	
//	/* Configures the priority grouping: 4 bits pre-emption priority */
//  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	GPIO_Config();
	TIM3_interrup(50);
	fep_dev_list_Sem = xSemaphoreCreateMutex();
	xSemaphoreGive(fep_dev_list_Sem);
	
#if USE_USART
	SER_Init(USART_BAUDRATE); 
	Serial_Sem = xSemaphoreCreateMutex();
	xSemaphoreGive(Serial_Sem);
#endif // USE_USART
	
	printd("tqk %d:", 100);
//	printf("tqk %d:", 100);
	
#if USE_BUZZER
	BuzzerInit();
#endif
	
	/* RTC timer */
#if USE_RTC
	RTC_Timer.Init();
#endif

	/* Buttons */
#if USE_BUTTONS
	BUTTON_API.Init();
#endif
	
#if (USE_ETHERNET)
	
	// load ethernet data from extern flash
//	SyncEthernetData(RESTORE);
	// 
	if (ethData._ip.addr == IPADDR_BROADCAST ||
		ethData._ip.addr == IPADDR_ANY ||
		!ip_addr_netmask_valid(&(ethData._subnet)))
	{
		IP4_ADDR(&(ethData._ip), IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
		IP4_ADDR(&(ethData._subnet), NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
		IP4_ADDR(&(ethData._gateway), GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	}
	
	ETH_BSP_Config();
	  
	// initial LwIP
	LwIP_Init();
			
#ifdef USE_DHCP
	/* Start DHCPClient */
	xTaskCreate(LwIP_DHCP_task, "DHCP", configMINIMAL_STACK_SIZE * 2, NULL,DHCP_TASK_PRIO, NULL);
#endif // USE_DHCP
	
#endif // USE_ETHERNET
	
#if USE_DALI
	DALI_Init();
#endif //USE_DALI

}

#if USE_ETHERNET
// for IP address
void ChangeStaticIP(void)
{
	if (netif_is_up(&xnetif))
	{
		netif_set_down(&xnetif);
		netif_set_addr(&xnetif, &(ethData._ip), &(ethData._subnet), &(ethData._gateway));
		// also change MAC address
		xnetif.hwaddr[5] = ip4_addr4(&ethData._ip.addr);
		ETH_MACAddressConfig(ETH_MAC_Address0, xnetif.hwaddr);
		netif_set_up(&xnetif);
	}
	else
	{
		netif_set_addr(&xnetif, &(ethData._ip), &(ethData._subnet), &(ethData._gateway));
		// also change MAC address
		xnetif.hwaddr[5] = ip4_addr4(&ethData._ip.addr);
		ETH_MACAddressConfig(ETH_MAC_Address0, xnetif.hwaddr);
	}
}

#endif
