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

void LPM_Delay(uint16_t n_cycles)
{
    USS_generateLPMDelay(&gUssSWConfig,USS_low_power_mode_option_low_power_mode_3, n_cycles);
}

