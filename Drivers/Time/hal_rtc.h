/*
 * main.c
 *
 *  Created on: Mar 2, 2015
 *      Author: kieutq
 */
#ifndef _HAL_RTC_h_
#define _HAL_RTC_h_

// include standard integer type
#include <inttypes.h> // C99

/* Define some useful data structure */
/* For reusable purpose, I redefine following structures. 
It compatible with STM32 RTC data struct on standard peripheral library */
struct Time
{
	uint8_t Hours;		// Value range from 0 to 23
	uint8_t Minutes;	// Value range from 0 to 59
	uint8_t Seconds;	// Value range form 0 to 59
	uint8_t padding; 	// padding, alway equal zero
};
typedef union 
{
	uint32_t 	Time_N; // integer number type of time
	struct Time Time_S;	// clearly structure members
} Time_t;

struct Date
{
	uint8_t DayOfWeeks;	// Value range from 1 to 7, which mean Mon to Sun
	uint8_t Months;		// Value range from 1 to 12
	uint8_t Days;		// Value range from 1 to 31
	uint8_t Years;		// Value range from 0 to 99
};
typedef union 
{
	uint32_t 	Date_N; // integer number type of date
	struct Date Date_S;	// clearly structure members
} Date_t;

typedef struct
{
	/* pointer-functions list */
	void 		(*Init)		(void);
	uint32_t 	(*GetTime)	(void);
	void 		(*SetTime)	(uint32_t);
	uint32_t 	(*GetDate)	(void);
	void 		(*SetDate)	(uint32_t);
	int32_t	(*DiffTime)	(uint32_t last, uint32_t curr);
} HAL_RTC_Class_t;

extern HAL_RTC_Class_t RTC_Timer;

#endif
