#include "fep_define.h"
#include "rs485.h"

// RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "drivers.h"

#include "lwip/sockets.h"
#include <string.h>

#define FEP_UDP_STACK			2*configMINIMAL_STACK_SIZE
#define FEP_UDP_PRIORITY		( tskIDLE_PRIORITY + 2 )

void FEP_UDP_Handler(void * pvParams);
extern SemaphoreHandle_t fep_dev_list_Sem;

void Create_FEP_UDP(void)
{
	xTaskCreate(FEP_UDP_Handler, "FEP UDP", 
				FEP_UDP_STACK, NULL,
				FEP_UDP_PRIORITY, NULL);
}

#define SERVER_PORT 1357
#define SERVER_IP_ADDR "192.168.1.111"

#define SOFTWARE_PACKET_LENGTH		5

unsigned long register_timeout, send_value_timeout;
#define REGISTER_TIMEOUT	100
#define SEND_VALUE_TIMEOUT	20

/*
dinh nghia so ket noi va dia chi server
so ket noi phuc vu toi da: 5
nguyen tat phuc vu: ket noi sau se duoc dua vao list, 
qua 5 ket noi se don list, loai bo ket noi dau tien
*/

#define MAX_SERVER_CONNECT		5
struct sockaddr_in fep_server_addr[MAX_SERVER_CONNECT];
uint8_t number_server=1;//so ket noi hien tai;

uint8_t buff_value[20];
uint16_t sum=0;

int SendFindMasterReturn(int socket_fd, struct sockaddr * addr, int addr_len);
int SendRegister(int socket_fd, struct sockaddr * addr, int addr_len, FEP_DEV_t dev);
int SendValue(int socket_fd, struct sockaddr * addr, int addr_len, FEP_DEV_t dev);
int SendGetValue(int socket_fd, struct sockaddr * addr, int addr_len);
void check_ipserver(struct sockaddr_in addr);//kiem tra ipserver vua nhan co trong list?, neu chua co ser add vao list

uint8_t count_send=0;

extern uint8_t Flag_SendUDP;

void FEP_UDP_Handler(void * pvParams)
{
	int client_socket_fd;
	u32_t addr_len;
	struct sockaddr_in server_addr, client_addr;
	uint8_t recv_buf[10];
	
	client_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client_socket_fd < 0)
	{
		if (xSemaphoreTake(Serial_Sem, 1) == pdTRUE) // 1ms timeout
		{
			printf("Creating client socket have been fail!");
			xSemaphoreGive(Serial_Sem);
		}
	}
	else
	{
		if (xSemaphoreTake(Serial_Sem, 10) == pdTRUE) // 10ms timeout
		{
			printf("creat clinet socket success\n");
			xSemaphoreGive(Serial_Sem);
		}
	}
	
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	client_addr.sin_port = htons(SERVER_PORT);
	
	if (bind(client_socket_fd, (struct sockaddr *)&client_addr, 
			 sizeof(client_addr)) < 0)
	{
		if (xSemaphoreTake(Serial_Sem, 10) == pdTRUE) // 10ms timeout
		{
			printf("cannot bind to server address");
			xSemaphoreGive(Serial_Sem);
		}
	}
//cau hinh ket noi server	
	memset((char *) &server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	
	if (inet_aton(SERVER_IP_ADDR, &server_addr.sin_addr) == 0) 
	{
		if (xSemaphoreTake(Serial_Sem, 10) == pdTRUE) // 10ms timeout
		{
			printf("Cannot create server address");
			xSemaphoreGive(Serial_Sem);
		}
		for(;;);
	}
	addr_len = sizeof (server_addr);
	fep_server_addr[0] = server_addr;

///////////////////////////////////////////////////	
	register_timeout = xTaskGetTickCount();
	send_value_timeout = xTaskGetTickCount();
	
	if (xSemaphoreTake(Serial_Sem, 10) == pdTRUE) // 10ms timeout
	{
		printf("udp task starting.....\n");
		xSemaphoreGive(Serial_Sem);
	}

//	while(1)
//	{
//		vTaskDelay(2);
//		if (xSemaphoreTake(fep_dev_list_Sem, 1) == pdTRUE) // 1ms timeout
//		{
//			if(fep_dev_list[1].new_data == ACTIVE)
//			{
//				fep_dev_list[1].new_data = INACTIVE;
//				recv_buf[0]=fep_dev_list[1].data;
//				recv_buf[1]=fep_dev_list[1].dataL;
//				sendto(client_socket_fd, recv_buf, 2, 0,(struct sockaddr *)&server_addr, addr_len);
//			}
//			xSemaphoreGive(fep_dev_list_Sem);
//		}
//	}
	while (1)
	{
		vTaskDelay(2);
		if(Flag_SendUDP==ACTIVE)
		{
			Flag_SendUDP = INACTIVE;
			SendGetValue(client_socket_fd, (struct sockaddr *)&server_addr, addr_len);
		}
		// process Find Master or Register Return from software
		if (recvfrom(client_socket_fd, recv_buf, SOFTWARE_PACKET_LENGTH, MSG_DONTWAIT,
			(struct sockaddr *)&server_addr, &addr_len) == SOFTWARE_PACKET_LENGTH)
		{
			FEP_Packet_t * tmpPacket = (FEP_Packet_t *)recv_buf;
			
			//check_ipserver(server_addr);
			if (PACKET_START(tmpPacket) == START_FINDMASTER)
			{
				if (PACKET_STATE(tmpPacket) == STATUS_ASK)
				{
					SendFindMasterReturn(client_socket_fd, 
								 (struct sockaddr *)&server_addr, addr_len);
				}
			}
		}
		
	}
	
}

void check_ipserver(struct sockaddr_in addr)
{
	char i;
	char next_ip=0;
	for(i=0;i<number_server;i++)
	{
		if(addr.sin_addr.s_addr == fep_server_addr[i].sin_addr.s_addr)
		{
			printf("trung ip...\n");
			next_ip=1;
		}
	}
	if(next_ip==0)
	{
		printf("phat hien ip moi....\n");
		if(number_server==MAX_SERVER_CONNECT)
		{
			for(i=0;i<number_server;i++)
			{
				
			}
		}
		fep_server_addr[number_server]=addr;
		number_server++;
	}
}
int SendFindMasterReturn(int socket_fd, struct sockaddr * addr, int addr_len)
{
	FEP_Packet_t tmpPacket;
	
	// create Find Master Return packet
	PACKET_SET(&tmpPacket, START_FINDMASTER, STATUS_RETURN,
			   0, 0, 0);
	
	// send to addr
	return sendto(socket_fd, &tmpPacket, PACKET_SIZE(&tmpPacket), 0,
			addr, addr_len);
}

int SendRegister(int socket_fd, struct sockaddr * addr, int addr_len, FEP_DEV_t dev)
{
	FEP_Packet_t tmpPacket;
	
	// Create register packet
	PACKET_SET(&tmpPacket, START_REGISTER, STATUS_ONLINE,
			   dev.dev_type, dev.dev_id, dev.status);
	
	// send to addr
	return sendto(socket_fd, &tmpPacket, PACKET_SIZE(&tmpPacket), 0,
			addr, addr_len);
}

int SendValue(int socket_fd, struct sockaddr * addr, int addr_len, FEP_DEV_t dev)
{
	FEP_Packet_t tmpPacket;
	
	// Create value packet
	PACKET_SET(&tmpPacket, START_GETVALUE, STATUS_ONLINE,
			   dev.dev_type, dev.dev_id, dev.data);
	
	// send to addr
	return sendto(socket_fd, &tmpPacket, PACKET_SIZE(&tmpPacket), 0,
			addr, addr_len);
}

int SendGetValue(int socket_fd, struct sockaddr * addr, int addr_len)
{
	uint8_t buff[100];
	uint8_t i,j;
//	
	buff[0] = 0x11;												//start+state
	j=0;
	if (xSemaphoreTake(fep_dev_list_Sem, 1) == pdTRUE) // 1ms timeout
	{
		for(i=0;i<6;i++)
		{
			if(fep_dev_list[i+1].status==ACTIVE)
			{
				buff[3+3*j+0] = fep_dev_list[i+1].dev_type + fep_dev_list[i+1].dev_id;
				buff[3+3*j+1] = fep_dev_list[i+1].data;
				buff[3+3*j+2] = fep_dev_list[i+1].dataL;
				j++;
			}
		}
		xSemaphoreGive(fep_dev_list_Sem);
	}
	
	buff[1] = 3*j + 4;				//size
	buff[2] = 0x00;												//device_type + device_id
	buff[buff[1]-1] = 0xFE;								//check sum
	
	// send to addr
	return sendto(socket_fd, buff, buff[1], 0,
			addr, addr_len);
}































