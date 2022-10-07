/*
 * utils.c
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */
#include "utils.h"
#include <stdbool.h>

int ms_counter = 0;

bool UT_delayms(ut_tmrDelay_t* p_timer, uint32_t p_ms){
    switch(p_timer->state){
        case(UT_TMR_DELAY_WAIT):
               if((ms_counter - p_timer->startValue) >= p_ms){
                    p_timer->state = UT_TMR_DELAY_INIT;
                    return true;
                }else{
                    p_timer->state = UT_TMR_DELAY_WAIT;
                    return false;
                }
            break;
        case (UT_TMR_DELAY_INIT):
            p_timer->startValue = ms_counter;
            p_timer->state = UT_TMR_DELAY_WAIT;
            return false;
            break;
    }
}



