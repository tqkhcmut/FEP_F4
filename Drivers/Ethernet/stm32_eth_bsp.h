/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_ETH_BSP_H
#define __STM32_ETH_BSP_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "drivers.h"

/* Includes ------------------------------------------------------------------*/
#if defined (USE_STM324xG_EVAL)
  #include "stm324xg_eval.h"
  #include "stm324xg_eval_lcd.h"

#elif defined (USE_STM324x7I_EVAL) 
  #include "stm324x7i_eval.h"
  #include "stm324x7i_eval_lcd.h"

#elif defined (USE_STM32F1x7xB_ELEK) 
	#include <stm32f4xx.h>
	 
#else
 #error "Please select first the board used in your application (in Project Options)"
#endif

#include "lwip/netif.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DP83848_PHY_ADDRESS       0x01 

/* Specific defines for EXTI line, used to manage Ethernet link status */
#define ETH_LINK_EXTI_LINE             EXTI_Line7
#define ETH_LINK_EXTI_PORT_SOURCE      EXTI_PortSourceGPIOE
#define ETH_LINK_EXTI_PIN_SOURCE       EXTI_PinSource7
#define ETH_LINK_EXTI_IRQn             EXTI9_5_IRQn 
/* PB7 */
#define ETH_LINK_PIN                   GPIO_Pin_7
#define ETH_LINK_GPIO_PORT             GPIOE
#define ETH_LINK_GPIO_CLK              RCC_AHB1Periph_GPIOE

/* Ethernet Flags for EthStatus variable */   
#define ETH_INIT_FLAG           0x01 /* Ethernet Init Flag */
#define ETH_LINK_FLAG           0x10 /* Ethernet Link Flag */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void  ETH_BSP_Config(void);
uint32_t Eth_Link_PHYITConfig(uint16_t PHYAddress);
void Eth_Link_EXTIConfig(void);
void Eth_Link_IT_task(void * pvParameters);
void Eth_Link_EXTIConfig(void);
void ETH_link_callback(struct netif *netif);

#ifdef __cplusplus
}
#endif

#endif

