#include <msp430.h> 
#include <FlowMeter.h>
#include <hal_lcd.h> //FOR DEBUGGING
#include <stdio.h>
#include <IQMathlib.h>
#include "utils.h"
#include <Communications.h>
#include "string.h"

/*
 * main.c
 */
_iq16 volumeFlowRate = 0;

int main(void)
{
    uint16_t reset_source = 0x00;
    int32_t flujo;
    // --------- System Setup -------------

    hal_system_Init();
	reset_source = hal_system_GetResetSource();
	    if(reset_source != 0x00)
	    {
	        __no_operation();
	    }

	hal_lcd_turnonLCD();
	__enable_interrupt();
	flowMeter_setup();
	Communications_setup();

	// --------- main program loop ----------
	while(1){

	    if(!UT_timer_delay()){

	        if(Communications_isActive()){
	            Communications_update();
	        }
	    }else{
	        //flujo = flowMeter_getVolumeFlowRate();

	        _iq16 flujo_int = _IQ16int(flujo); // casteo flujo a int
	        char lcd_output[6] = "";

	        /* IMPORTANTE: el compilador de TI, por defecto, reduce la cantidad de funcionalidades de sprintf
	         * a las funcionalidades minimas basicas para ahorrar memoria. Para hacer este truco de castear y
	         * agregar ceros a la izquierda tuve que modificar un parametro del compilador que incorpora todas
	         * las funcionalidades de sprintf. Esto ocupa memoria! solo lo uso ahora para debugear con el display
	         * TODO: MODIFICAR EL PARAM "--printf_support" A "minimal" ENTRANDO EN PROPIEDADES->MSP430 COMPILER->ADVANCED OPTIONS->LANGUAGE OPTIONS
	         */
	        sprintf(lcd_output, "%06d", flujo_int); // casteo el flujo de int a string

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
	    }
	}


