#include "fep_process.h"
#include "fep_define.h"
#include "rs485.h"
#include "drivers.h"

FEP_DEV_t fep_dev_list[MAX_DEVICES];	// fix array list, use more resource but faster access
FEP_DEV_t fep_dev_list_tmp[MAX_DEVICES];

int dev_count;	// devices count

// quan
uint8_t number_device=0;
uint8_t number_device_tmp=0;

// mutex used to guard common data
SemaphoreHandle_t fep_common_access;

// following queue use by fep_rs485 by call extern declaration
#define FEP_QUEUEBUFFER_SIZE 20
QueueHandle_t FEPProcess_QueueBuffer;

extern void Create_FEP_UDP(void);
extern void Create_FEP_RS485(void);

// quan
extern void Create_FEP_RS485_GETVALUE(void);

void CreateFEPProcess(void)
{
	// initial data and driver functions
	dev_count = 0;
	
	// data access mutex
	fep_common_access = xSemaphoreCreateMutex();
	if (fep_common_access == NULL)
	{
#if USE_USART
		if (xSemaphoreTake(Serial_Sem, 1) == pdTRUE) // 1ms timeout
		{
			printf("fep_common_access cannot created.\r\n");
			xSemaphoreGive(Serial_Sem);
		}
#endif
		for(;;); // trap the CPU, for debug only
	}
	
	// create receiver data queue buffer
	// create queue interface
	FEPProcess_QueueBuffer = xQueueCreate(FEP_QUEUEBUFFER_SIZE, sizeof(struct FEP_Packet));
	if (FEPProcess_QueueBuffer == 0)
	{
#if USE_USART
		if (xSemaphoreTake(Serial_Sem, 1) == pdTRUE) // 1ms timeout
		{
			printf("FEPProcess: packet queue buffer cannot created.\r\n");
			xSemaphoreGive(Serial_Sem);
		}
#endif
		for(;;); // trap the CPU, for debug only
	}
	RS485_API.Init(FEPProcess_QueueBuffer);
	RS485_API.dir(UART_485_DIR_RECE);
	
	// create udp process task
	Create_FEP_UDP();
	
	// create rs485 process task
//	Create_FEP_RS485();
	
	// quan
	Create_FEP_RS485_GETVALUE();
	
	Create_FEP_RS485();
}

