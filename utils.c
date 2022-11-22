/*
 * utils.c
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */
#include "utils.h"
#include <stdbool.h>
#include "ussSwLib.h"
#include <msp430.h>
#include "USS_userConfig.h"

#define ACLK_FREQUENCY 32768 //clock frequency, used to calibrate delay.

UT_TMR_DELAY_STATE timer_delay;


bool UT_timer_delay(){
    if (timer_delay == UT_TMR_DELAY_INIT){
        timer_delay = UT_TMR_DELAY_WAIT;
        return true;
    }else{
        return false;
    }

}

void UT_timer_setDelayDuration(){
    __no_operation();
}

void LPM_Delay(uint16_t n_cycles)
{
    USS_generateLPMDelay(&gUssSWConfig,USS_low_power_mode_option_low_power_mode_3, n_cycles);
}

