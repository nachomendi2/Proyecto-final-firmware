#include <msp430.h> 
#include <FlowMeter.h>
#include <stdio.h>
#include <IQMathlib.h>
#include "utils.h"
#include <Communications.h>
#include "string.h"
#include <stdbool.h>
#include "ValveControl.h"
#include "Memory.h"
#include <PressureSensor.h>
#include "gpio.h"



//#define __AFE_EXT_3v3__ // remove this to use external AFE
/*
 * main.c
 */
extern ut_tmrDelay_t wakeup_timer;

int blink_led_setup(void) {
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0);
    return 0;
}

int blink_led(void) {
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    return 0;
}

int main(void)
{
    uint16_t reset_source = 0x00;
    wakeup_timer.restartAfterCompletion = true;
    // --------- System Setup -------------

    hal_system_Init();
    Memory_loadBackup();
	reset_source = hal_system_GetResetSource();
	    if(reset_source != 0x00)
	    {
	        __no_operation();
	    }
	__enable_interrupt();
	Communications_setup();
	//PressureSensor_setup();
	flowMeter_setup();
	//blink_led_setup();

	//flowMeter_setMeasurementTimeInterval(5);

	valveControl_setup();

	// --------- main program loop ----------
	while(1){
	    Communications_update();
	    if(!UT_timer_delay(&wakeup_timer)){
	        // enter LPM3:
	        if (!Communications_isActive()){
	            __low_power_mode_3();
	            continue;
	        }

	    }else{
	        valveControl_update();
	        //blink_led();
	        //PressureSensor_update();
	        flowMeter_update();
	        Memory_backupData();
	    }
    }
}


