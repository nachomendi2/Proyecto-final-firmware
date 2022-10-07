#include <msp430.h> 
#include <FlowMeter.h>
#include <hal_lcd.h> //FOR DEBUGGING
#include <stdio.h>
#include <utils.h>


/*
 * main.c
 */
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

	hal_lcd_turnonLCD();

	__enable_interrupt();
	flowMeter_setup();


	// --------- main program loop ----------
	while(1){

	    float flujo = flowMeter_getVolumeFlowRate()/100;
	    char lcd_output[6] = "000000";
	    sprintf(lcd_output, "%d",(int) flujo);

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

	    // Simple & ineficient delay for quick test (TODO: remove this!)
	    uint16_t i;
	    for(i = 0; i<10000; i++){
	        __no_operation();
	    }

	}
}
