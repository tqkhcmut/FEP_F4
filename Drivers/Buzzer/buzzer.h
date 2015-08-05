#ifndef _buzzer_h_
#define _buzzer_h_

#include "drivers.h"

#ifndef USE_BUZZER
#define USE_BUZZER 0
#endif

#if USE_BUZZER

extern QueueHandle_t BuzzerQueue;

#if defined(BUZZER_NORMAL)
struct BuzzerData
{
	uint8_t State;
	uint16_t Life;
};
#elif defined(BUZZER_ADVANCE)
struct BuzzerData
{
	uint16_t Freq;
	uint16_t Amp;
	uint16_t Life
};
#else
#error "Buzzer mode was not defined."
#endif

void BuzzerInit(void);

// extern some simple functions
void BuzzerTick(int time_ms);	// turn burrer on in period time then off
void BuzzerOn(void);	// turn buzzer on
void BuzzerOff(void);	// turn buzzer off

#endif // USE_BUZZER
#endif
