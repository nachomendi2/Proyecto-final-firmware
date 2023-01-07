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

uint8_t UT_mod2div(uint8_t *dividend, uint8_t divisor, uint8_t pol_length){
    // SOURCE: https://www.devcoons.com/crc8/
        uint8_t remainder = 0x00;
        uint8_t extract;
        uint8_t sum;
       for(uint8_t i=0;i<pol_length;i++)
       {
          extract = *dividend; //*(dividend+i);
          for (uint8_t tempI = 8; tempI; tempI--)
          {
             sum = (remainder ^ extract) & 0x01;
             remainder >>= 1;
             if (sum)
                remainder ^= divisor;
             extract >>= 1;
          }
          dividend++;
       }
    return remainder;
}

