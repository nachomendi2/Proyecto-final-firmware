#include <msp430.h> 
#include <FlowMeter.h>
#include <hal_lcd.h> //FOR DEBUGGING
#include <stdio.h>
#include <IQMathlib.h>
#include "utils.h"
#include <Communications.h>
#include "string.h"
#include <stdbool.h>
#include "ValveControl.h"
#include "Memory.h"
#include <PressureSensor.h>


//#define __AFE_EXT_3v3__ // remove this to use external AFE
/*
 * main.c
 */
extern ut_tmrDelay_t wakeup_timer;

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

	hal_lcd_turnonLCD();
	Communications_setup();
	PressureSensor_setup();
    __enable_interrupt();
	flowMeter_setup();
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
	        PressureSensor_update();
	        flowMeter_update();
	        // displayOnLCD();
	        Memory_backupData();
	    }
    }
}

// Only for debug, can be removed on release:
void displayOnLCD(){
    char lcd_output[6] = "";

    /* IMPORTANTE: el compilador de TI, por defecto, reduce la cantidad de funcionalidades de sprintf
     * a las funcionalidades minimas basicas para ahorrar memoria. Para hacer este truco de castear y
     * agregar ceros a la izquierda tuve que modificar un parametro del compilador que incorpora todas
     * las funcionalidades de sprintf. Esto ocupa memoria! solo lo uso ahora para debugear con el display
     * TODO: MODIFICAR EL PARAM "--printf_support" A "minimal" ENTRANDO EN PROPIEDADES->MSP430 COMPILER->ADVANCED OPTIONS->LANGUAGE OPTIONS
     */
    float f_totalizer = flowMeter_getTotalizer();

    sprintf(lcd_output, "%06f", f_totalizer); // casteo el flujo de int a string

    // Display on LCD:
    uint8_t j=0;
    for(j = 0; j<6; j++){
        switch(j){
            case 0: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_1 );
                break;
            case 1: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_2 );
                break;
            case 2: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_3 );
                break;
            case 3: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_4 );
                break;
            case 4: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_5 );
                break;
            case 5: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_6 );
                break;
            }
        }

}


