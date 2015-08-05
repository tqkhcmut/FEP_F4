#include "fep_define.h"
#include "rs485.h"

// RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stdio.h"

#include "fep_gpio.h"
#include "rs485.h"

#define FEP_RS485_STACK			2*configMINIMAL_STACK_SIZE
#define FEP_RS485_PRIORITY		( tskIDLE_PRIORITY + 2 )

extern uint8_t Resum_getvalue;

extern QueueHandle_t FEPProcess_QueueBuffer;
extern SemaphoreHandle_t fep_dev_list_Sem;
void FEP_RS485_Handler(void * pvParams);

void Create_FEP_RS485(void)
{
	xTaskCreate(FEP_RS485_Handler, "FEP RS485", 
				FEP_RS485_STACK, NULL,
				FEP_RS485_PRIORITY, NULL);
}
/*
update by quan:
-function: check device of list
return:
index device: if have device in list
255: no device in list
*/
uint8_t check_device(FEP_Packet_t tmpPacket)
{
	char i;
	for(i=0;i<number_device_tmp;i++)
	{
		if ((fep_dev_list_tmp[i].dev_type 		== PACKET_DEV_TYPE(&tmpPacket) &&
			fep_dev_list_tmp[i].dev_longaddr 		== PACKET_VALUE(&tmpPacket)) || //longaddr = value
			(fep_dev_list_tmp[i].dev_type 			== PACKET_DEV_TYPE(&tmpPacket) &&
			fep_dev_list_tmp[i].dev_id		 			== PACKET_DEV_ID(&tmpPacket))
			)
		{
			return i;
		}
	}
	return 255;
}
/*
update by quan:
-function: return packet register device
-tham so index: la index cua thiet bi trong fep_dev_list_tmp
*/
void return_register(uint8_t index)
{
	FEP_Packet_t packet;

	packet.start_state 	= 0x0C;
	packet.size 				= 0x05;
	packet.dev_info 		= (fep_dev_list_tmp[index].dev_type) + (fep_dev_list_tmp[index].dev_id);
	packet.value			 	= fep_dev_list_tmp[index].dev_longaddr;
	packet.checksum			= 0xFE;
	RS485_API.SendPacket(&packet, 1);
}

/*
update by quan:
-function: reset slave
-tham so:
	device_type: loai thiet bi
	device_id: id thiet bi
*/
void reset_slave(char device_type, char device_id)
{
	FEP_Packet_t packet;
	
	packet.start_state 	= 0x29;
	packet.size 				= 0x05;
	packet.dev_info 		= device_type<<3 | device_id;
	packet.value			 	= 0x00;
	packet.checksum			= 0xFE;
	RS485_API.SendPacket(&packet, 1);
}

/*
update by quan:
-function: getfreeid for device
-tham so vao: dev_type: kieu cua thiet bi muon getfreeid
-giai thuat:
	tim tat ca thiet bi co cung dev_type va get ve id cua no dua vao buff_id
	cap phat 1 so khong trung voi mang buff_id bat dau tu 0
	=> thiet bi dc cap id tu 0 tro len
-return:
	freeid of dev_type
*/
char getFreeID(char dev_type)
{
	int i=0;
	uint8_t index=0;
	uint8_t buff_id[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	if(number_device_tmp==0) return 0;
	for(i=0;i<number_device_tmp;i++)
	{
		if (fep_dev_list_tmp[i].dev_type == dev_type)
		{
			buff_id[index]=fep_dev_list_tmp[i].dev_id;
			index++;
		}
	}
	
	for(i=0;i<index;i++)
	{
		if(i != buff_id[0] && i != buff_id[1] && 
		   i != buff_id[2] && i != buff_id[3] &&
		   i != buff_id[4] && i != buff_id[5] &&
		   i != buff_id[6] && i != buff_id[7])
		{
			return i;
		}
	}
	return index%7;
}

void debug_listdevice(void)
{
	int i;
	printf("------ list debug ----------------\n");
	printf("number device = %d\n", number_device_tmp);
	for(i=0;i<number_device_tmp;i++)
	{
		printf("----------device %d----------\n",i);
//		printf("	dev_longaddr		= %x\n", fep_dev_list_tmp[i].dev_longaddr);
//		printf("	slave_register		= %d\n", fep_dev_list_tmp[i].slave_register);
//		printf("	dev_type			= %s\n", fep_dev_list_tmp[i].dev_type==DTYPE_DISTANCE?"DTYPE_DISTANCE":
//												 fep_dev_list_tmp[i].dev_type==DTYPE_TEMPERATURE?"DTYPE_TEMPERATURE":
//												 fep_dev_list_tmp[i].dev_type==DTYPE_PRESSURE?"DTYPE_PRESSURE":
//												 fep_dev_list_tmp[i].dev_type==DTYEP_PH?"DTYEP_PH":"NO DEVICE CODE");
		printf("	dev_id				= %d\n", fep_dev_list_tmp[i].dev_id);
//		printf("	status				= %d\n", fep_dev_list_tmp[i].status);
//		printf("	need_register		= %d\n", fep_dev_list_tmp[i].need_register);
//		printf("	need_send_data		= %d\n", fep_dev_list_tmp[i].need_send_data);
//		printf("	data				= %d\n", fep_dev_list_tmp[i].data);
	}
}
extern void ask_value(uint8_t index);
extern uint8_t data_buff[10];
extern uint8_t Flag_SendUDP;

void FEP_RS485_Handler(void * pvParams)
{
	FEP_Packet_t tmpPacket;
	
	printf("fep task starting...\n");
//	LED_RUN_TOLGE;
	
	while(1)
	{
		if (xQueueReceive(FEPProcess_QueueBuffer, &tmpPacket, NULL) == pdTRUE)
		{
			switch (PACKET_START(&tmpPacket))
			{
//				LED_RUN_TOLGE;
				case START_GETVALUE://goi value se cap nhat vao danh sach that
					if (PACKET_STATE(&tmpPacket) == STATUS_RETURN)
					{
							if (xSemaphoreTake(fep_dev_list_Sem, 1) == pdTRUE) // 1ms timeout
							{
								fep_dev_list[PACKET_DEV_ID(&tmpPacket)].dev_type 		= PACKET_DEV_TYPE(&tmpPacket);
								fep_dev_list[PACKET_DEV_ID(&tmpPacket)].dev_id   		= PACKET_DEV_ID(&tmpPacket);
								if((((uint16_t)data_buff[3]<<8) | (uint16_t)data_buff[4]) != 0xFFFF )
								{
									fep_dev_list[PACKET_DEV_ID(&tmpPacket)].new_data	= ACTIVE;
									fep_dev_list[PACKET_DEV_ID(&tmpPacket)].data 			= data_buff[3];
									fep_dev_list[PACKET_DEV_ID(&tmpPacket)].dataL			= data_buff[4];
								}
								else
								{
//									LED_RUN_TOLGE;
								}
								fep_dev_list[PACKET_DEV_ID(&tmpPacket)].rece_count	=0;//xoa bo dem rece
								fep_dev_list[PACKET_DEV_ID(&tmpPacket)].status = ACTIVE;
								
								xSemaphoreGive(fep_dev_list_Sem);
							}
					}
					break;
				default:
					break;
			}
		}
//		vTaskDelay(1);
	}
}

