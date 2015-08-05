#include <stm32f4xx.h>

#include "FreeRTOS.h"
#include "task.h"

#include "Tasks/LED_Run_Task.h"


#include "FEP/fep_process.h"
#include "FEP/fep_gpio.h"
#include "FEP/rs485.h"


#define MAIN_TASK_PRIO   ( tskIDLE_PRIORITY + 3 )

void MainTask(void * pvParameters);


int main()
{
	/* Configures the priority grouping: 4 bits pre-emption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	// start main task
	xTaskCreate(MainTask, "Main", configMINIMAL_STACK_SIZE * 2, NULL, MAIN_TASK_PRIO, NULL);
	
	/* Start the scheduler. */
	vTaskStartScheduler();

	for(;;); //trap the cpu
}


void MainTask(void * pvParameters)
{	
//	RCC_ClocksTypeDef RCC_Clocks;
		
	// Initial hardware
	DriversInit();
	
		
	BuzzerTick(1000);
		
//	RCC_GetClocksFreq(&RCC_Clocks);
	
		
	CreateFEPProcess();
	//
//	LED_Run_Task_Create();
	
	for( ;; )
  	{
    	vTaskDelete(NULL); // exit by itself
  	}
}

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	// free
}

void vApplicationMallocFailedHook( void )
{
	// if you are enter this function, it mean memory allocation 
	// have been failed.
	// This cause may be you have not enough heap memory. Expand
	// FreeRTOS memory on FreeRTOSConfig.h may fix this broblem.
	
	for(;;); // halt the CPU
}

void delay_ms(uint32_t nCount)
{
	vTaskDelay(nCount);
}
