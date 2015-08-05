#include "fep_define.h"
#include "rs485.h"

// RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stdio.h"

#define FEP_RS485_GETVALUE_STACK			2*configMINIMAL_STACK_SIZE
#define FEP_RS485_GETVALUE_PRIORITY		( tskIDLE_PRIORITY + 2 )

#define MODE_LIST					0
#define MODE_LIST_TEMP		1

uint8_t Resum_getvalue = TIME_RESUM;//resum task khi co goi register den
uint8_t mode=MODE_LIST_TEMP;

uint8_t Flag_GetData = INACTIVE;//co quan ly viec tick goi gui data, duoc kich hoat trong time3
uint8_t Flag_SendUDP = INACTIVE;//co bao gui data qua UDP

extern SemaphoreHandle_t fep_dev_list_Sem;

void FEP_RS485_GETVALUE_Handler(void * pvParams);

void Create_FEP_RS485_GETVALUE(void)
{
	xTaskCreate(FEP_RS485_GETVALUE_Handler, "FEP RS485 GETVALUE", 
				FEP_RS485_GETVALUE_STACK, NULL,
				FEP_RS485_GETVALUE_PRIORITY, NULL);
}

/*
update by quan:
-function: send packet ask value
-tham so index: la index cua thiet bi trong fep_dev_list_tmp
*/
void ask_value(uint8_t index)
{
	FEP_Packet_t packet;
	
	RS485_API.sel(index);
	packet.start_state 	= 0x13;
	packet.size 				= 0x05;
	packet.dev_info 		= (0x1F<<3) + index;
	packet.value			 	= 0x00;
	packet.checksum			= 0xFE;
	packet.next					= NULL;
	RS485_API.SendPacket(&packet, 1);
}
void update_slave(uint8_t index)
{
	FEP_Packet_t packet;

	packet.start_state 	= 0x23;
	packet.size 				= 0x05;
	packet.dev_info 		= (fep_dev_list_tmp[index].dev_type) + (fep_dev_list_tmp[index].dev_id);
	packet.value			 	= 0x00;
	packet.checksum			= 0xFE;
	packet.next					= NULL;
	RS485_API.SendPacket(&packet, 1);
}
void FEP_RS485_GETVALUE_Handler(void * pvParams)
{
	int i;
	
	printf("fep_rs485_getvalue task starting...\n");
	
	while(1)
	{
		vTaskDelay(1);
		if(Flag_GetData==ACTIVE)
		{
			for(i=1;i<=6;i++)
			{
				ask_value(i);
				vTaskDelay(4);
				if (xSemaphoreTake(fep_dev_list_Sem, 1) == pdTRUE) // 1ms timeout
				{
					fep_dev_list[i].rece_count++;
					xSemaphoreGive(fep_dev_list_Sem);
				}
			}
//			for(i=1;i<6;i++)
//			{
//				if((fep_dev_list[i].status==ACTIVE) && (fep_dev_list[i].new_data==INACTIVE))
//				{
//					ask_value(i);
//					vTaskDelay(4);
//					if (xSemaphoreTake(fep_dev_list_Sem, 1) == pdTRUE) // 1ms timeout
//					{
//						fep_dev_list[i].rece_count++;
//						xSemaphoreGive(fep_dev_list_Sem);
//					}
//				}
//			}
			
			//vTaskDelay(20);//se tinh lai time delay nay de dat sample tot nhat
			//kiem tra so goi tin gui, neu qua so lan cho phep se disconnet thiet bi
			for(i=1;i<=6;i++)
			{
				fep_dev_list[i].new_data = INACTIVE;
				if(fep_dev_list[i].rece_count>10)//20 goi tin 
				{
					fep_dev_list[i].status = INACTIVE;
				}
			}
			Flag_GetData = INACTIVE;
			Flag_SendUDP = ACTIVE;
		}
	}
//	while(1)
//	{
//		while(Resum_getvalue>0)
//		{
//			vTaskDelay(1000);
//			Resum_getvalue--;
//			mode = MODE_LIST_TEMP;//chay mode list temp de update slave
//			count=0;
//		}
//		if(mode == MODE_LIST_TEMP)
//		{
//			for(i=0;i<number_device_tmp;i++)
//			{
//				update_slave(i);
//				vTaskDelay(4);
//				fep_dev_list_tmp[i].update_count++;
//			}
//			vTaskDelay(10);//se tinh lai time delay nay de dat sample tot nhat
//			//kiem tra so goi tin gui, neu qua so lan cho phep se disconnet thiet bi
//			for(i=0;i<number_device_tmp;i++)
//			{
//				if(fep_dev_list_tmp[i].update_count>20)//10 goi tin 
//				{
//					fep_dev_list_tmp[i].status = INACTIVE;
//				}
//			}
//			count++;
//			if(count>=25)
//			{
//				//cap nhat danh sach qua list thuc
//				j=0;
//				for(i=0;i<number_device_tmp;i++)
//				{
//					if(fep_dev_list_tmp[i].status == ACTIVE)
//					{
//						fep_dev_list[j] = fep_dev_list_tmp[i];
//						j++;
//					}
//				}
//				//cap nhat bien number_device
//				number_device = j;
//				number_device_tmp = j;
//				//cap nhat lai list temp
//				for(i=0;i<number_device;i++)
//				{
//					fep_dev_list_tmp[i] = fep_dev_list[i];
//				}
//				//debug list
//				printf("#####################################\n");
//				for(i=0;i<number_device;i++)
//				{
//					printf("----------dev %d--\n",i);
//					printf("id = %d\n", fep_dev_list[i].dev_id);
//					printf("st = %d\n", fep_dev_list[i].status);
//				}
//				printf("#####################################\n");
//				mode = MODE_LIST;
//				vTaskDelay(100);
//				count=0;
//			}
//		}
//		else //mode list, cap nhat value
//		{
//				for(i=0;i<number_device;i++)
//				{
//					ask_value(i);
//					vTaskDelay(4);
//					if (xSemaphoreTake(fep_dev_list_Sem, 1) == pdTRUE) // 1ms timeout
//					{
//						fep_dev_list[i].rece_count++;
//						xSemaphoreGive(fep_dev_list_Sem);
//					}
//				}
//				vTaskDelay(20);//se tinh lai time delay nay de dat sample tot nhat
//				//kiem tra so goi tin gui, neu qua so lan cho phep se disconnet thiet bi
//				for(i=0;i<number_device;i++)
//				{
//					if(fep_dev_list[i].rece_count>20)//20 goi tin 
//					{
//						fep_dev_list[i].status = INACTIVE;
//						device_disconnet = 1;
//					}
//				}
//				if(device_disconnet == 1)//co thiet bi disconnect, cap nhat lai danh sach
//				{
//					device_disconnet = 0;
//					j=0;
//					for(i=0;i<number_device;i++)
//					{
//						if(fep_dev_list[i].status == ACTIVE)
//						{
//							fep_dev_list_tmp[j] = fep_dev_list[i];
//							j++;
//						}
//					}
//					number_device=j;
//					number_device_tmp = j;
//					//cap nhat lai list temp
//					for(i=0;i<number_device;i++)
//					{
//						fep_dev_list[i] = fep_dev_list_tmp[i];
//					}
//					//debug list
//					printf("#####################################\n");
//					for(i=0;i<number_device;i++)
//					{
//						printf("----------dev %d--\n",i);
//						printf("id = %d\n", fep_dev_list[i].dev_id);
//						printf("st = %d\n", fep_dev_list[i].status);
//					}
//					printf("#####################################\n");
//				}
//		}
//	}
}

