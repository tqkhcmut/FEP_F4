#ifndef _my_hex_file_h_
#define _my_hex_file_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

/* Record data, remove ':' character and convert to byte, not ACCII format */
struct Record
{
	// the length of data
	uint8_t length; 
	
	// the data of record, contain all field in normal record except ':' character
	uint8_t * data; 
	
	// the pointer point to next record
	struct Recort * next; 
};

/* Hex File structure used to store to external flash */
struct HexFile
{
	// the length of file data equal total size of each struct record
	uint32_t FileLength;
	
	// struct pointer point to record list
	struct Record * record;
};

#ifdef __cplusplus
}
#endif

#endif

