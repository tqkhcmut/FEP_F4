#include "intel_hex.h"

// be carefull that this function work only with upper-cast character
uint8_t convertToByteArray(char * charData, uint8_t * destData, uint8_t len)
{
	
	uint8_t res = 0;
	uint8_t i = 0;
	uint8_t hByte = 0, lByte = 0;
	// 
	// check start character. It may be ':' may be nothing
	if (*charData == ':')
	{
		// skip it
		charData++;
		// decrease len by 1
		len--;
	}
	if (len % 2 != 0)
	{
		// invalid length of the string
		return res;
	}
	for (i = 0; i < len; i++)
	{
		if (*charData != '\0' && *charData != '\r' && *charData != '\n')
		{
			hByte = *(charData) > '9' ? *(charData)-'A' + 10 : *(charData)-'0';
			lByte = *(charData + 1) > '9' ? *(charData + 1)-'A' + 10 : *(charData + 1) - '0';

			*(destData++) = (hByte << 4) | lByte;

			res++;
			charData += 2;
		}
		else
		{
			break;
		}
	}

	return res;
}
uint8_t checksum_check(uint8_t * data, uint8_t length)
{
	uint8_t tmpChksum = 0;
	uint8_t i = 0;
	for (i = 0; i < length-1; i++)
	{
		tmpChksum += *(data + i);
	}
	// reverse bits of checksum, we got implement 1
	tmpChksum ^= 0xff;
	// add 1 to get the implement 2
	tmpChksum++;
	// check the valid of checksum field
	if (tmpChksum != *(data + length-1))
	{
		return 1;
	}
	else
		return 0;
}
uint32_t getBaseAddress(uint8_t * data)
{
	uint32_t res = 0;
	switch (getRecordType(data))
	{
	case DataRecordType:
		break;
	case EndOfFileRecordType:
		break;
	case ExtendedSegmentAddressRecordType:
		res |= (*(data + DATA_POS) << 20);
		res |= (*(data + DATA_POS + 1) << 12);
		break;
	case StartSegmentAddressRecordType:
		res |= (*(data + DATA_POS) << 24);
		res |= (*(data + DATA_POS + 1) << 16);
		res |= (*(data + DATA_POS + 2) << 8);
		res |= (*(data + DATA_POS + 3));
		break;
	case ExtendedLinearAddressRecordType:
		res |= (*(data + DATA_POS) << 24);
		res |= (*(data + DATA_POS + 1) << 16);
		break;
	case StartLinearAddressRecordType:
		res |= (*(data + DATA_POS) << 24);
		res |= (*(data + DATA_POS + 1) << 16);
		res |= (*(data + DATA_POS + 2) << 8);
		res |= (*(data + DATA_POS + 3));
		break;
	default:
		break;
	}
	return res;
}

RecordType_t getRecordType(uint8_t * data)
{
	return (RecordType_t)*(data + RECORD_TYPE_POS);
}


uint16_t getLoadOffset(uint8_t * data)
{
	uint16_t res = 0;
	res |= *(data + LOAD_OFFSET_POS) << 8;
	res |= *(data + LOAD_OFFSET_POS + 1);
	return res;
}


