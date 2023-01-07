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
#include <hal_timer_a.h>

ut_tmrDelay_t wakeup_timer;
extern ValveControl_Module valve;

int hal_timer_a_InitWakeUpTimer(){

    // Configure TA3 for delay between measurements:
    Timer_A_initUpModeParam timer_delay_config;
    timer_delay_config.timerPeriod = 512;
    timer_delay_config.timerClear = TIMER_A_DO_CLEAR;
    timer_delay_config.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    timer_delay_config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    timer_delay_config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    timer_delay_config.startTimer = true;

    Timer_A_initUpMode(__MSP430_BASEADDRESS_TA3__, &timer_delay_config);
    Timer_A_enableInterrupt(__MSP430_BASEADDRESS_TA3__);

    return 0;
}

void hal_timer_a_InitValveDelay(){

    // Configure TA4 for timing valve command pulse duration
    Timer_A_initUpModeParam timer_valve_config;
    timer_valve_config.timerPeriod = 25800;
    timer_valve_config.timerClear = TIMER_A_DO_CLEAR;
    timer_valve_config.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    timer_valve_config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_4;
    timer_valve_config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    timer_valve_config.startTimer = true;

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
    wakeup_timer.state = UT_TMR_DELAY_INIT;
    __bic_SR_register_on_exit(CPUOFF); // wake up from LPM
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
    valve.command_Pulse_Delay.state = UT_TMR_DELAY_INIT;
    Timer_A_stop(__MSP430_BASEADDRESS_TA4__);
    Timer_A_disableInterrupt(__MSP430_BASEADDRESS_TA4__);
    __bic_SR_register_on_exit(CPUOFF); // wake up from LPM
}

inline void hal_timer_a_setWakeUptimerPeriod(uint16_t time){
    Timer_A_setCompareValue(
            __MSP430_BASEADDRESS_TA3__,
            TIMER_A_CAPTURECOMPARE_REGISTER_0,
            time
            );
}
