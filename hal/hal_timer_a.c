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
#include <ValveControl.h>

UT_TMR_DELAY_STATE wakeup_timer;
extern ValveControl_Module valve;

int hal_timer_a_InitWakeUpTimer(){

    // Configure TA3 for delay between measurements:
    Timer_A_initUpModeParam timer_delay_config;
    timer_delay_config.timerPeriod = 32768;
    timer_delay_config.timerClear = TIMER_A_DO_CLEAR;
    timer_delay_config.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    timer_delay_config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timer_delay_config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    timer_delay_config.startTimer = true;

    Timer_A_initUpMode(__MSP430_BASEADDRESS_TA3__, &timer_delay_config);
    Timer_A_enableInterrupt(__MSP430_BASEADDRESS_TA3__);

    return 0;
}

void hal_timer_a_InitValveDelay(){
    // Configure TA4 for timing valve command pulse duration
    Timer_A_initUpModeParam timer_valve_config;
    timer_valve_config.timerPeriod = 32768;
    timer_valve_config.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    timer_valve_config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_4;
    timer_valve_config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;

    Timer_A_initUpMode(__MSP430_BASEADDRESS_TA4__, &timer_valve_config);
    Timer_A_enableInterrupt(__MSP430_BASEADDRESS_TA4__);

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
    wakeup_timer = UT_TMR_DELAY_INIT;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER4_A1_VECTOR
__interrupt void timerA4_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER4_A1_VECTOR))) timerA4_ISR (void)
#else
#error Compiler not supported!
#endif
{

    Timer_A_clearTimerInterrupt(__MSP430_BASEADDRESS_TA4__);
    valve.command_pulse_delay = UT_TMR_DELAY_INIT;
}