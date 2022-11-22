/*
 * hal_timer_a.c
 *
 *  Created on: 7 oct. 2022
 *      Author: ignaciomendizabal
 */

#include <msp430.h>
#include <timer_a.h>
#include <stdint.h>
#include <utils.h>

extern UT_TMR_DELAY_STATE timer_delay;

int hal_timer_a_Init(){

    // By default, configure timer to interrupt every 1 second.
    Timer_A_initUpModeParam timer_config;
    timer_config.timerPeriod = 65000;
   // timer_config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timer_config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    timer_config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    timer_config.startTimer = true;

    Timer_A_initUpMode(__MSP430_BASEADDRESS_TA3__, &timer_config);
    Timer_A_enableInterrupt(__MSP430_BASEADDRESS_TA3__);
    return 0;
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER3_A1_VECTOR
__interrupt void timerA3_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER3_A1_VECTOR))) timerA3_ISR (void)
#else
#error Compiler not supported!
#endif
{

    Timer_A_clearTimerInterrupt(__MSP430_BASEADDRESS_TA3__);
    timer_delay = UT_TMR_DELAY_INIT;
    TA3CCR0 = 65000;
}
