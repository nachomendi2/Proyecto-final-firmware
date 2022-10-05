#include <msp430.h> 
#include <FlowMeter.h>


float flujo;
/*
 * main.c
 */
int main(void)
{
	// Initializes the basic functionality of the system
	hal_system_Init();

	__enable_interrupt();
	flowMeter_setup();

	while(1){

	    flujo = flowMeter_measure();
	    short i;
	    for(i = 0; i<1000; i++){
	        __no_operation();
	    }
	}
	
}
