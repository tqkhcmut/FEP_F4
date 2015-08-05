/**
*	@Author:		kieutq@hotmail.com
*	@Description:	Intel Hex File Utility
*	@Version:		0.1
*	@
*/

#ifndef _intel_hex_h_
#define _intel_hex_h_

#ifdef __cplusplus
extern  "C" {
#endif

#include <inttypes.h>

#ifndef NULL
#define NULL 0
#endif
	
	/* Record data, remove ':' character and convert to byte, not ACCII format */
struct Record
{
	// the length of data
	uint8_t length; 
	
	// the data of record, contain all field in normal record except ':' character
	uint8_t * data; 
	
	// the pointer point to next record
	struct Record * next; 
};

/* Hex File structure used to store to external flash */
struct HexFile
{
	// the length of file data equal total size of each struct record
	uint32_t RecordCount;
	
	// struct pointer point to record list
	struct Record * records;
};
	
	
#define RECORD_LEN_POS	0
#define LOAD_OFFSET_POS	1
#define RECORD_TYPE_POS	3
#define DATA_POS		4

	typedef enum
	{
		DataRecordType = 0,
		EndOfFileRecordType = 1,
		ExtendedSegmentAddressRecordType = 2,
		StartSegmentAddressRecordType = 3,
		ExtendedLinearAddressRecordType = 4,
		StartLinearAddressRecordType = 5
	} RecordType_t;

	//
	uint8_t convertToByteArray(char * charData, uint8_t * destData, uint8_t len);
	//
	uint8_t checksum_check(uint8_t * data, uint8_t length);
	//
	uint32_t getBaseAddress(uint8_t * data);
	// 
	uint16_t getLoadOffset(uint8_t * data);
	//
	RecordType_t getRecordType(uint8_t * data);
	//
	//uint8_t program(uint8_t * data);

	// extern method for program flash
	/*extern uint8_t FlashInit(void);
	extern uint8_t FlashProgramWord(uint32_t address, uint32_t data);
	extern uint8_t FlashProgramHalfWord(uint32_t address, uint16_t data);
	extern uint8_t FlashProgramByte(uint32_t address, uint8_t data);
	extern void FlashUnLock(void);
	extern void FlashLock(void);*/

#ifdef __cplusplus
}
#endif


#endif
