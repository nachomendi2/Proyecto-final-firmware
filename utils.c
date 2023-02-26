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
#include <USS_userConfig.h>

#define ACLK_FREQUENCY 32768 //clock frequency, used to calibrate delay.

bool UT_timer_delay(ut_tmrDelay_t *timer_delay){
    if (timer_delay->state == UT_TMR_DELAY_INIT){

        if(timer_delay->restartAfterCompletion){
            timer_delay->state = UT_TMR_DELAY_WAIT;
        }

        return true;
    }else{
        return false;
    }

}


