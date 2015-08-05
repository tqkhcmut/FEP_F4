#ifndef _fep_define_h_
#define _fep_define_h_

#include "FreeRTOS.h"
#include "semphr.h"

//define start code
#define START_REGISTER		(0x01<<3)
#define START_GETVALUE		(0x02<<3)
#define START_FINDMASTER	(0x03<<3)
#define START_UPDATESLAVE	(0x04<<3)

//define state
#define STATUS_ONLINE		(0x01<<0)
#define STATUS_OFFLINE		(0x02<<0)
#define STATUS_ASK			(0x03<<0)
#define STATUS_RETURN		(0x04<<0)

//define device type
#define DTYPE_DISTANCE		(0x01<<3)
#define DTYPE_TEMPERATURE	(0x02<<3)
#define DTYPE_PRESSURE		(0x03<<3)
#define DTYEP_PH			(0x04<<3)

//define check sum
#define CHECKSUM			0xFE

// FE protocol structure
typedef struct FEP_Packet
{
	uint8_t start_state;	
	uint8_t size; 			// alway equal 0x05
	uint8_t dev_info;		
	uint8_t value;			
	uint8_t checksum;		// alway equal 0xFE
	uint8_t valueL;
	struct FEP_Packet * next;
} FEP_Packet_t;

// some useful marcros
// use with FEP_Packet pointer
#define PACKET_VALID(x) 	((x)->size == 5 && (x)->checksum == CHECKSUM)
#define PACKET_SIZE(x) 		((x)->size)
#define PACKET_START(x) 	((x)->start_state&0xF8)
#define PACKET_STATE(x) 	((x)->start_state&0x07)
#define PACKET_DEV_TYPE(x) 	((x)->dev_info&0xF8)
#define PACKET_DEV_ID(x) 	((x)->dev_info&0x07)
#define PACKET_VALUE(x) 	((x)->value)
#define PACKET_SET(x, _start, _state, _dev_type, _dev_id, _value) \
{ \
	(x)->start_state = _start + _state; \
		(x)->size = 0x05; \
			(x)->dev_info = _dev_type + _dev_id; \
				(x)->value = _value; \
					(x)->checksum = CHECKSUM; \
}

// following defines useful for FEP_DEV structure
#ifndef INACTIVE
#define INACTIVE 0
#else
#waning "INACTIVE have been defined"
#endif
#ifndef ACTIVE
#define ACTIVE 1
#else
#waning "ACTIVE have been defined"
#endif
typedef struct FEP_DEV
{
	uint8_t dev_type;
	uint8_t dev_id;
	uint8_t status;
	uint8_t need_register;
	uint8_t data;
	uint8_t dataL;
	
	//quan: bo sung
	uint8_t dev_longaddr;
	uint8_t slave_register;//=ACTIVE neu co device moi va chua tra loi, sau khi tra loi device se INACTIVE
	//quan: bo sung 
	uint8_t rece_count;//bien dem so goi rece, dung de phat hien device disconnet
	uint8_t update_count;//tuong tu rece_count, bien nay qua ly so goi update_slave, de phat hien su ton tai cua thiet bi
	uint8_t new_data;//bien quan ly data moi
	
	struct FEP_DEV * next; 		// use for dynamic list
} FEP_DEV_t;


// common data, declare in fep_process.c
extern uint8_t number_device;
extern uint8_t number_device_tmp;

#define MAX_DEVICES 30
#define TIME_RESUM	3
extern FEP_DEV_t fep_dev_list[MAX_DEVICES];	// fix array list, use more resource but faster access
extern FEP_DEV_t fep_dev_list_tmp[MAX_DEVICES];
//extern int dev_count;

// mutex used to guard common data
extern SemaphoreHandle_t fep_common_access;


#endif
