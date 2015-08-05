/**
* @file
* Ethernet Interface Skeleton
*
*/

/*
* Copyright (c) 2001-2004 Swedish Institute of Computer Science.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
* SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
*
* This file is part of the lwIP TCP/IP stack.
*
* Author: Adam Dunkels <adam@sics.se>
*
*/

/*
* This file is a skeleton for developing Ethernet network interface
* drivers for lwIP. Add code to the low_level functions and do a
* search-and-replace for the word "ethernetif" to replace it with
* something that better describes your network interface.
*/

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/timers.h"
#include "netif/etharp.h"
#include "lwip/err.h"
#include "ethernetif.h"

#include "main.h"
#include "drivers.h"
#include <string.h>

typedef struct{
u32 length;
u32 buffer;
ETH_DMADESCTypeDef *descriptor;
}FrameTypeDef;

typedef struct  {
  __IO ETH_DMADESCTypeDef *FS_Rx_Desc;          /*!< First Segment Rx Desc */
  __IO ETH_DMADESCTypeDef *LS_Rx_Desc;          /*!< Last Segment Rx Desc */
  __IO uint32_t  Seg_Count;                     /*!< Segment count */
} ETH_DMA_Rx_Frame_infos;
  
FrameTypeDef ETH_RxPkt_ChainMode(void);
u32 ETH_GetCurrentTxBuffer(void);
u32 ETH_TxPkt_ChainMode(u16 FrameLength);
uint32_t ETH_Prepare_Transmit_Descriptors(u16 FrameLength);

#define netifMTU                                (1500)
#define netifINTERFACE_TASK_STACK_SIZE		( 350 )
#define netifINTERFACE_TASK_PRIORITY		( configMAX_PRIORITIES - 1 )
#define netifGUARD_BLOCK_TIME			( 250 )
/* The time to block waiting for input. */
#define emacBLOCK_TIME_WAITING_FOR_INPUT	( ( portTickType ) 100 )

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'

#define  ETH_DMARxDesc_FrameLengthShift           16
#define  ETH_ERROR              ((u32)0)
#define  ETH_SUCCESS            ((u32)1)

static struct netif *s_pxNetIf = NULL;
xSemaphoreHandle s_xSemaphore = NULL;

#define ETH_RXBUFNB        4
#define ETH_TXBUFNB        2

/* Ethernet Rx & Tx DMA Descriptors */
ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];

/* Ethernet Receive buffers  */
uint8_t Rx_Buff[ETH_RXBUFNB][ETH_MAX_PACKET_SIZE]; 

/* Ethernet Transmit buffers */
uint8_t Tx_Buff[ETH_TXBUFNB][ETH_MAX_PACKET_SIZE]; 

/* Global pointers to track current transmit and receive descriptors */
extern ETH_DMADESCTypeDef  *DMATxDescToSet;
extern ETH_DMADESCTypeDef  *DMARxDescToGet;

/* Global pointer for last received frame infos */
ETH_DMA_Rx_Frame_infos DMA_RX_FRAME_infos;


static void ethernetif_input( void * pvParameters );
static void arp_timer(void *arg);


/**
* In this function, the hardware should be initialized.
* Called from ethernetif_init().
*
* @param netif the already initialized lwip network interface structure
*        for this ethernetif
*/
static void low_level_init(struct netif *netif)
{
	uint32_t i;
	
	/* set netif MAC hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;
	
	/* set netif MAC hardware address */
	netif->hwaddr[0] =  MAC_ADDR0;
	netif->hwaddr[1] =  MAC_ADDR1;
	netif->hwaddr[2] =  MAC_ADDR2;
	netif->hwaddr[3] =  MAC_ADDR3;
	netif->hwaddr[4] =  MAC_ADDR4;
	netif->hwaddr[5] =  MAC_ADDR5;
	
	/* set netif maximum transfer unit */
	netif->mtu = 1500;
	
	/* Accept broadcast address and ARP traffic */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
	
	s_pxNetIf =netif;
	
	/* create binary semaphore used for informing ethernetif of frame reception */
	if (s_xSemaphore == NULL)
	{
		vSemaphoreCreateBinary(s_xSemaphore);
		xSemaphoreTake( s_xSemaphore, 0);
	}
	
	/* initialize MAC address in ethernet MAC */ 
	ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr); 
	
	/* Initialize Tx Descriptors list: Chain Mode */
	ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
	/* Initialize Rx Descriptors list: Chain Mode  */
	ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
	
	/* Enable Ethernet Rx interrrupt */
	{ 
		for(i=0; i<ETH_RXBUFNB; i++)
		{
			ETH_DMARxDescReceiveITConfig(&DMARxDscrTab[i], ENABLE);
		}
	}
	
#ifdef CHECKSUM_BY_HARDWARE
	/* Enable the checksum insertion for the Tx frames */
	{
		for(i=0; i<ETH_TXBUFNB; i++)
		{
			ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
		}
	} 
#endif
	
	/* create the task that handles the ETH_MAC */
	xTaskCreate(ethernetif_input, "Eth_if", netifINTERFACE_TASK_STACK_SIZE, NULL,
				netifINTERFACE_TASK_PRIORITY,NULL);
		
		/* Enable MAC and DMA transmission and reception */
		ETH_Start();   
}


/**
* This function should do the actual transmission of the packet. The packet is
* contained in the pbuf that is passed to the function. This pbuf
* might be chained.
*
* @param netif the lwip network interface structure for this ethernetif
* @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
* @return ERR_OK if the packet could be sent
*         an err_t value if the packet couldn't be sent
*
* @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
*       strange results. You might consider waiting for space in the DMA queue
*       to become availale since the stack doesn't retry to send a packet
*       dropped because of memory failure (except for the TCP timers).
*/

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	static xSemaphoreHandle xTxSemaphore = NULL;
	struct pbuf *q;
	u8 *buffer ;
	__IO ETH_DMADESCTypeDef *DmaTxDesc;
	uint16_t framelength = 0;
	uint32_t bufferoffset = 0;
	uint32_t byteslefttocopy = 0;
	uint32_t payloadoffset = 0;
	
	if (xTxSemaphore == NULL)
	{
		vSemaphoreCreateBinary (xTxSemaphore);
	}
	
	if (xSemaphoreTake(xTxSemaphore, netifGUARD_BLOCK_TIME))
	{
		DmaTxDesc = DMATxDescToSet;
		buffer = (u8 *)(DmaTxDesc->Buffer1Addr);
		bufferoffset = 0;
		
		for(q = p; q != NULL; q = q->next) 
		{
			if((DmaTxDesc->Status & ETH_DMATxDesc_OWN) != (u32)RESET)
			{
				goto error;
			}
			
			/* Get bytes in current lwIP buffer  */
			byteslefttocopy = q->len;
			payloadoffset = 0;
			
			/* Check if the length of data to copy is bigger than Tx buffer size*/
			while( (byteslefttocopy + bufferoffset) > ETH_MAX_PACKET_SIZE )
			{
				/* Copy data to Tx buffer*/
				memcpy( (u8_t*)((u8_t*)buffer + bufferoffset), (u8_t*)((u8_t*)q->payload + payloadoffset), (ETH_MAX_PACKET_SIZE - bufferoffset) );
				
				/* Point to next descriptor */
				DmaTxDesc = (ETH_DMADESCTypeDef *)(DmaTxDesc->Buffer2NextDescAddr);
				
				/* Check if the buffer is available */
				if((DmaTxDesc->Status & ETH_DMATxDesc_OWN) != (u32)RESET)
				{
					goto error;
				}
				
				buffer = (u8 *)(DmaTxDesc->Buffer1Addr);
				
				byteslefttocopy = byteslefttocopy - (ETH_MAX_PACKET_SIZE - bufferoffset);
				payloadoffset = payloadoffset + (ETH_MAX_PACKET_SIZE - bufferoffset);
				framelength = framelength + (ETH_MAX_PACKET_SIZE - bufferoffset);
				bufferoffset = 0;
			}
			
			/* Copy the remaining bytes */
			memcpy( (u8_t*)((u8_t*)buffer + bufferoffset), (u8_t*)((u8_t*)q->payload + payloadoffset), byteslefttocopy );
			bufferoffset = bufferoffset + byteslefttocopy;
			framelength = framelength + byteslefttocopy;
		}
		
		/* Prepare transmit descriptors to give to DMA*/
		ETH_Prepare_Transmit_Descriptors(framelength);
		
		/* Give semaphore and exit */
	error:
		
		xSemaphoreGive(xTxSemaphore);
	}
	
	return ERR_OK;
}

/**
* Should allocate a pbuf and transfer the bytes of the incoming
* packet from the interface into the pbuf.
*
* @param netif the lwip network interface structure for this ethernetif
* @return a pbuf filled with the received packet (including MAC header)
*         NULL on memory error
*/
static struct pbuf * low_level_input(struct netif *netif)
{
	struct pbuf *p= NULL, *q;
	u32_t len;
	FrameTypeDef frame;
	u8 *buffer;
	__IO ETH_DMADESCTypeDef *DMARxDesc;
	uint32_t bufferoffset = 0;
	uint32_t payloadoffset = 0;
	uint32_t byteslefttocopy = 0;
	uint32_t i=0;  
	
	/* get received frame */
	frame = ETH_RxPkt_ChainMode();
	
	/* Obtain the size of the packet and put it into the "len" variable. */
	len = frame.length;
	buffer = (u8 *)frame.buffer;
	
	if (len > 0)
	{
		/* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
		p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	}
	
	if (p != NULL)
	{
		DMARxDesc = frame.descriptor;
		bufferoffset = 0;
		for(q = p; q != NULL; q = q->next)
		{
			byteslefttocopy = q->len;
			payloadoffset = 0;
			
			/* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size*/
			while( (byteslefttocopy + bufferoffset) > ETH_MAX_PACKET_SIZE )
			{
				/* Copy data to pbuf*/
				memcpy( (u8_t*)((u8_t*)q->payload + payloadoffset), (u8_t*)((u8_t*)buffer + bufferoffset), (ETH_MAX_PACKET_SIZE - bufferoffset));
				
				/* Point to next descriptor */
				DMARxDesc = (ETH_DMADESCTypeDef *)(DMARxDesc->Buffer2NextDescAddr);
				buffer = (unsigned char *)(DMARxDesc->Buffer1Addr);
				
				byteslefttocopy = byteslefttocopy - (ETH_MAX_PACKET_SIZE - bufferoffset);
				payloadoffset = payloadoffset + (ETH_MAX_PACKET_SIZE - bufferoffset);
				bufferoffset = 0;
			}
			
			/* Copy remaining data in pbuf */
			memcpy( (u8_t*)((u8_t*)q->payload + payloadoffset), (u8_t*)((u8_t*)buffer + bufferoffset), byteslefttocopy);
			bufferoffset = bufferoffset + byteslefttocopy;
		}
		
		/* Release descriptors to DMA */
		DMARxDesc =frame.descriptor;
		
		/* Set Own bit in Rx descriptors: gives the buffers back to DMA */
		for (i=0; i<DMA_RX_FRAME_infos.Seg_Count; i++)
		{  
			DMARxDesc->Status = ETH_DMARxDesc_OWN;
			DMARxDesc = (ETH_DMADESCTypeDef *)(DMARxDesc->Buffer2NextDescAddr);
		}
		
		/* Clear Segment_Count */
		DMA_RX_FRAME_infos.Seg_Count =0;
		/* added for test*/
	}
	
	/* When Rx Buffer unavailable flag is set: clear it and resume reception */
	if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)  
	{
		/* Clear RBUS ETHERNET DMA flag */
		ETH->DMASR = ETH_DMASR_RBUS;
		/* Resume DMA reception */
		ETH->DMARPDR = 0;
	}
	return p;
}


/**
* This function is the ethernetif_input task, it is processed when a packet 
* is ready to be read from the interface. It uses the function low_level_input() 
* that should handle the actual reception of bytes from the network
* interface. Then the type of the received packet is determined and
* the appropriate input function is called.
*
* @param netif the lwip network interface structure for this ethernetif
*/
void ethernetif_input( void * pvParameters )
{
	struct pbuf *p;
	
	for( ;; )
	{
		if (xSemaphoreTake( s_xSemaphore, emacBLOCK_TIME_WAITING_FOR_INPUT)==pdTRUE)
		{
		TRY_GET_NEXT_FRAME:
			p = low_level_input( s_pxNetIf );
			if   (p != NULL)
			{
				if (ERR_OK != s_pxNetIf->input( p, s_pxNetIf))
				{
					pbuf_free(p);
				}
				else
				{
					goto TRY_GET_NEXT_FRAME;
				}
			}
		}
	}
}

/**
* Should be called at the beginning of the program to set up the
* network interface. It calls the function low_level_init() to do the
* actual setup of the hardware.
*
* This function should be passed as a parameter to netif_add().
*
* @param netif the lwip network interface structure for this ethernetif
* @return ERR_OK if the loopif is initialized
*         ERR_MEM if private data couldn't be allocated
*         any other err_t on error
*/
err_t ethernetif_init(struct netif *netif)
{
	LWIP_ASSERT("netif != NULL", (netif != NULL));
	
#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */
	
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	
	netif->output = etharp_output;
	netif->linkoutput = low_level_output;
	
	/* initialize the hardware */
	low_level_init(netif);
	
	etharp_init();
	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
	
	return ERR_OK;
}


static void arp_timer(void *arg)
{
	etharp_tmr();
	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}


/*******************************************************************************
* Function Name  : ETH_RxPkt_ChainMode
* Description    : Receives a packet.
* Input          : None
* Output         : None
* Return         : frame: farme size and location
*******************************************************************************/
FrameTypeDef ETH_RxPkt_ChainMode(void)
{ 
  u32 framelength = 0;
  FrameTypeDef frame = {0,0}; 

  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (u32)RESET)
  {	
	frame.length = ETH_ERROR;

    if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)  
    {
      /* Clear RBUS ETHERNET DMA flag */
      ETH->DMASR = ETH_DMASR_RBUS;
      /* Resume DMA reception */
      ETH->DMARPDR = 0;
    }

	/* Return error: OWN bit set */
    return frame; 
  }
  
  if(((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (u32)RESET) && 
     ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (u32)RESET) &&  
     ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (u32)RESET))  
  {      
    /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
    framelength = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARxDesc_FrameLengthShift) - 4;
	
	/* Get the addrees of the actual buffer */
	frame.buffer = DMARxDescToGet->Buffer1Addr;	
  }
  else
  {
    /* Return ERROR */
    framelength = ETH_ERROR;
  }

  frame.length = framelength;


  frame.descriptor = DMARxDescToGet;
  
  /* Update the ETHERNET DMA global Rx descriptor with next Rx decriptor */      
  /* Chained Mode */    
  /* Selects the next DMA Rx descriptor list for next buffer to read */ 
  DMARxDescToGet = (ETH_DMADESCTypeDef*) (DMARxDescToGet->Buffer2NextDescAddr);    
  
  /* Return Frame */
  return (frame);  
}

/*******************************************************************************
* Function Name  : ETH_TxPkt_ChainMode
* Description    : Transmits a packet, from application buffer, pointed by ppkt.
* Input          : - FrameLength: Tx Packet size.
* Output         : None
* Return         : ETH_ERROR: in case of Tx desc owned by DMA
*                  ETH_SUCCESS: for correct transmission
*******************************************************************************/
u32 ETH_TxPkt_ChainMode(u16 FrameLength)
{   
  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (u32)RESET)
  {  
	/* Return ERROR: OWN bit set */
    return ETH_ERROR;
  }
        
  /* Setting the Frame Length: bits[12:0] */
  DMATxDescToSet->ControlBufferSize = (FrameLength & ETH_DMATxDesc_TBS1);

  /* Setting the last segment and first segment bits (in this case a frame is transmitted in one descriptor) */    
  DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;

  /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
  DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;

  /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
  if ((ETH->DMASR & ETH_DMASR_TBUS) != (u32)RESET)
  {
    /* Clear TBUS ETHERNET DMA flag */
    ETH->DMASR = ETH_DMASR_TBUS;
    /* Resume DMA transmission*/
    ETH->DMATPDR = 0;
  }
  
  /* Update the ETHERNET DMA global Tx descriptor with next Tx decriptor */  
  /* Chained Mode */
  /* Selects the next DMA Tx descriptor list for next buffer to send */ 
  DMATxDescToSet = (ETH_DMADESCTypeDef*) (DMATxDescToSet->Buffer2NextDescAddr);    


  /* Return SUCCESS */
  return ETH_SUCCESS;   
}

/**
  * @brief  Prepares DMA Tx descriptors to transmit an ethernet frame
  * @param  FrameLength : length of the frame to send
  * @retval error status
  */
uint32_t ETH_Prepare_Transmit_Descriptors(u16 FrameLength)
{   
  uint32_t buf_count =0, size=0,i=0;
  ETH_DMADESCTypeDef *DMATxDesc;

  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (u32)RESET)
  {  
    /* Return ERROR: OWN bit set */
    return ETH_ERROR;
  }

  DMATxDesc = DMATxDescToSet;
  
  if (FrameLength > ETH_MAX_PACKET_SIZE)
  {
    buf_count = FrameLength/ETH_MAX_PACKET_SIZE;
    if (FrameLength%ETH_MAX_PACKET_SIZE) buf_count++;
  }
  else buf_count =1;

  if (buf_count ==1)
  {
    /*set LAST and FIRST segment */
    DMATxDesc->Status |=ETH_DMATxDesc_FS|ETH_DMATxDesc_LS;
    /* Set frame size */
    DMATxDesc->ControlBufferSize = (FrameLength & ETH_DMATxDesc_TBS1);
    /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
    DMATxDesc->Status |= ETH_DMATxDesc_OWN;
    DMATxDesc= (ETH_DMADESCTypeDef *)(DMATxDesc->Buffer2NextDescAddr);
  }
  else
  {
    for (i=0; i< buf_count; i++)
    {
      /* Clear FIRST and LAST segment bits */
      DMATxDesc->Status &= ~(ETH_DMATxDesc_FS | ETH_DMATxDesc_LS);
      
      if (i==0) 
      {
        /* Setting the first segment bit */
        DMATxDesc->Status |= ETH_DMATxDesc_FS;  
      }

      /* Program size */
      DMATxDesc->ControlBufferSize = (ETH_MAX_PACKET_SIZE & ETH_DMATxDesc_TBS1);
      
      if (i== (buf_count-1))
      {
        /* Setting the last segment bit */
        DMATxDesc->Status |= ETH_DMATxDesc_LS;
        size = FrameLength - (buf_count-1)*ETH_MAX_PACKET_SIZE;
        DMATxDesc->ControlBufferSize = (size & ETH_DMATxDesc_TBS1);
      }

      /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
      DMATxDesc->Status |= ETH_DMATxDesc_OWN;

      DMATxDesc = (ETH_DMADESCTypeDef *)(DMATxDesc->Buffer2NextDescAddr);
    }
  }
  
  DMATxDescToSet = DMATxDesc;

  /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
  if ((ETH->DMASR & ETH_DMASR_TBUS) != (u32)RESET)
  {
    /* Clear TBUS ETHERNET DMA flag */
    ETH->DMASR = ETH_DMASR_TBUS;
    /* Resume DMA transmission*/
    ETH->DMATPDR = 0;
  }

  /* Return SUCCESS */
  return ETH_SUCCESS;   
}

