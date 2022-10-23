#include <msp430.h> 
#include <FlowMeter.h>
#include <hal_lcd.h> //FOR DEBUGGING
#include <stdio.h>
#include <IQMathlib.h>
#include "utils.h"


/*
 * main.c
 */
_iq16 flujo_int;
int32_t flujo;

int main(void)
{
    uint16_t reset_source = 0x00;


    // --------- System Setup -------------

	hal_system_Init();

	reset_source = hal_system_GetResetSource();
	    if(reset_source != 0x00)
	    {
	        __no_operation();
	    }

	__enable_interrupt();
	flowMeter_setup();
	hal_lcd_turnonLCD();

	// --------- main program loop ----------
	while(1){
	    flujo = flowMeter_getVolumeFlowRate();


	    flujo_int = _IQ16int(flujo);
        char lcd_output[6] = "000000";

        sprintf(lcd_output, "%d", flujo_int);

        short j = 0;
        for(j = 0; j<6; j++){
            switch(j){
                case 0: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_6 );
                    break;
                case 1: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_5 );
                    break;
                case 2: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_4 );
                    break;
                case 3: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_3 );
                    break;
                case 4: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_2 );
                    break;
                case 5: hal_lcd_showChar(lcd_output[j], HAL_LCD_DIGIT_1 );
                    break;
                }
            }

        LPM_Delay(20000);
	    }

}
