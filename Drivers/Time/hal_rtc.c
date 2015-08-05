/*
* main.c
*
*  Created on: Mar 2, 2015
*      Author: kieutq
*/
#include "hal_rtc.h"
#include "stm32f4xx.h"

#define RTC_CLOCK_SOURCE_LSE
#define FIRST_DATA          0x32F2
#define RTC_BKP_DR_NUMBER   0x14

// private data
Time_t _time = {0};
Date_t _date = {0};

static void 		HAL_RTC_Init	(void)
{
	/* initial hardware */
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the PWR APB1 Clock Interface */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);
	
	
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != FIRST_DATA)
	{
		RTC_InitTypeDef   RTC_InitStructure;
		__IO uint32_t uwAsynchPrediv = 0;
		__IO uint32_t uwSynchPrediv = 0;
		uint32_t uwErrorIndex = 0;
		uint32_t uwIndex = 0;
		
		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		
		/* Allow access to RTC */
		PWR_BackupAccessCmd(ENABLE);
		
#if defined (RTC_CLOCK_SOURCE_LSI)  /* LSI used as RTC source clock*/
		/* The RTC Clock may varies due to LSI frequency dispersion. */
		/* Enable the LSI OSC */
		RCC_LSICmd(ENABLE);
		
		/* Wait till LSI is ready */ 
		while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		{
		}
		
		/* Select the RTC Clock Source */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		/* ck_spre(1Hz) = RTCCLK(LSI) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
		uwSynchPrediv = 0xFF;
		uwAsynchPrediv = 0x7F;
		
#elif defined (RTC_CLOCK_SOURCE_LSE) /* LSE used as RTC source clock */
		/* Enable the LSE OSC */
		RCC_LSEConfig(RCC_LSE_ON);
		
		/* Wait till LSE is ready */ 
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
		{
		}
		
		/* Select the RTC Clock Source */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
		uwSynchPrediv = 0xFF;
		uwAsynchPrediv = 0x7F;
		
#else
#error Please select the RTC Clock source inside the main.c file
#endif /* RTC_CLOCK_SOURCE_LSI */
		
		/* Enable the RTC Clock */
		RCC_RTCCLKCmd(ENABLE);
		
		/* Wait for RTC APB registers synchronisation */
		RTC_WaitForSynchro();
		
		/* Write to the first RTC Backup Data Register */
		RTC_WriteBackupRegister(RTC_BKP_DR0, FIRST_DATA);
		
		
		/* Set the Time */
		_time.Time_S.Hours   = 8;
		_time.Time_S.Minutes = 0;
		_time.Time_S.Seconds = 0;
		
		/* Set the Date */
		_date.Date_S.Months 	= 3;
		_date.Date_S.Days 		= 2; 
		_date.Date_S.Years		= 15;
		_date.Date_S.DayOfWeeks = RTC_Weekday_Friday;
		
		/* Calendar Configuration */
		RTC_InitStructure.RTC_AsynchPrediv = uwAsynchPrediv;
		RTC_InitStructure.RTC_SynchPrediv =  uwSynchPrediv;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
		RTC_Init(&RTC_InitStructure);
		
		/* Set Current Time and Date */
		RTC_SetTime(RTC_Format_BIN, (RTC_TimeTypeDef*)&_time); 
		RTC_SetDate(RTC_Format_BIN, (RTC_DateTypeDef*)&_date);
		
		/* Configure the RTC Wakeup Clock source and Counter (Wakeup event each 1 second) */
		RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
		RTC_SetWakeUpCounter(0x7FF);
		
		/* Enable the Wakeup Interrupt */
		RTC_ITConfig(RTC_IT_WUT, ENABLE);
		
		/* Enable Wakeup Counter */
		RTC_WakeUpCmd(ENABLE);
		
		/*  Backup SRAM ***************************************************************/
		/* Enable BKPRAM Clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
		
		/* Write to Backup SRAM with 32-Bit Data */
		for (uwIndex = 0x0; uwIndex < 0x1000; uwIndex += 4)
		{
			*(__IO uint32_t *) (BKPSRAM_BASE + uwIndex) = uwIndex;
		}
		/* Check the written Data */
		for (uwIndex = 0x0; uwIndex < 0x1000; uwIndex += 4)
		{
			if ((*(__IO uint32_t *) (BKPSRAM_BASE + uwIndex)) != uwIndex)
			{
				uwErrorIndex++;
			}
		}
		while(uwErrorIndex); // loop for error debug
		
		
		/* Enable the Backup SRAM low power Regulator to retain it's content in VBAT mode */
		PWR_BackupRegulatorCmd(ENABLE);
		
		/* Wait until the Backup SRAM low power Regulator is ready */
		while(PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET)
		{
		}
		
		/* RTC Backup Data Registers **************************************************/
		/* Write to RTC Backup Data Registers */
		for (uwIndex = 0; uwIndex < RTC_BKP_DR_NUMBER; uwIndex++)
		{
			RTC_WriteBackupRegister(uwIndex, FIRST_DATA + (uwIndex * 0x5A));
		}
	}
	else
	{
		RTC_WaitForSynchro();
		RTC_ClearITPendingBit(RTC_IT_WUT);
	}
}

static uint32_t 	HAL_RTC_GetTime	(void)
{
	RTC_GetTime(RTC_Format_BIN, (RTC_TimeTypeDef *)&_time);
	return _time.Time_N;
}
static void 		HAL_RTC_SetTime	(uint32_t time)
{
	_time.Time_N = time;	
	RTC_SetTime(RTC_Format_BIN, (RTC_TimeTypeDef*)&_time); 
	RTC_WaitForSynchro();
}
static uint32_t 	HAL_RTC_GetDate	(void)
{
	RTC_GetDate(RTC_Format_BIN, (RTC_DateTypeDef *)&_date);
	return _date.Date_N;
}
static void 		HAL_RTC_SetDate	(uint32_t date)
{
	_date.Date_N = date;	
	RTC_SetDate(RTC_Format_BIN, (RTC_DateTypeDef*)&_date);
	RTC_WaitForSynchro();
}

static int32_t HAL_RTC_DiffTime (uint32_t last, uint32_t curr)
{
	int32_t res = 0;
	res += ((Time_t *)&curr)->Time_S.Seconds - ((Time_t *)&last)->Time_S.Seconds;
	res += (((Time_t *)&curr)->Time_S.Minutes - ((Time_t *)&last)->Time_S.Minutes)*60;
	res += (((Time_t *)&curr)->Time_S.Hours - ((Time_t *)&last)->Time_S.Hours)*3600;
	return res;
}


HAL_RTC_Class_t RTC_Timer =
{
	.Init 		= HAL_RTC_Init,
	.SetTime 	= HAL_RTC_SetTime,
	.GetTime 	= HAL_RTC_GetTime,
	.SetDate 	= HAL_RTC_SetDate,
	.GetDate 	= HAL_RTC_GetDate,
	.DiffTime	= HAL_RTC_DiffTime
};

