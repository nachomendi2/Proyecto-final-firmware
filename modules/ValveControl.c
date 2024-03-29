/*
 * ValveControl.c
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */

#include "ValveControl.h"
#include <hal_timer_a.h>
#include <Communications.h>

ValveControl_Module valve;

// DEPRECATED:
bool ValveReceiveOrder ()
{
    if (GPIO_getInputPinValue(GPIO_PORT_PJ, GPIO_PIN0))
    {
        return false;
    } else
    {
        return true;
    }
}

inline ValveState_t valveControl_getValveState(){
    return valve.state;
}

void valveControl_setup()
{
    // Initial state should be loaded from FRAM, if no state loaded (init state) assume it's open
    if (valve.state == VALVE_INIT_STATE){
        valve.state = VALVE_OPEN_STATE;
        valveControl_open();
    }

    // Configure GPIO for controlling the valve (one pin for opening the valve and one for closing it)
    GPIO_setAsOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN3
        );

    GPIO_setAsOutputPin(
        GPIO_PORT_P7,
        GPIO_PIN0
        );

    // By default, set GPIO in LOW
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P1,
            GPIO_PIN3
            );

    GPIO_setOutputLowOnPin(
            GPIO_PORT_P7,
            GPIO_PIN0
            );

}

// UNUSED
void valveControl_toggle(uint8_t SelectedPin)
{
    /*
     EL PIN ES EL PRIMER N�MERO DE LA SIGUIENTE TABLA
     CONECTOR J10 DEL EVM:

     1: Tx5VEn  -> P1.1
     2: XPB0    -> P7.0
     3: RxPwr   -> P2.2
     4: RxEn    -> P6.7
     5: TxSel   -> P2.3
     6: RxSel1  -> P3.6
     7: TxPwr   -> P1.0
     8: RxSel2  -> P3.7
    */
    switch (SelectedPin) {
    case 1:
        GPIO_toggleOutputOnPin(
            GPIO_PORT_P1,
            GPIO_PIN1
            );
        break;
    case 2:
        GPIO_toggleOutputOnPin(
            GPIO_PORT_P7,
            GPIO_PIN0
            );
        break;
    default:
        break;
    }
}

bool valveControl_open()
{
    /*
     EL PIN ES EL PRIMER N�MERO DE LA SIGUIENTE TABLA
     CONECTOR J10 DEL EVM:

     1: Tx5VEn  -> P1.1
     2: XPB0    -> P7.0
     3: RxPwr   -> P2.2
     4: RxEn    -> P6.7
     5: TxSel   -> P2.3
     6: RxSel1  -> P3.6
     7: TxPwr   -> P1.0
     8: RxSel2  -> P3.7
    */

    // Only try to open the valve if it's closed (commented for debugging)
    //if (valve.state != VALVE_CLOSED_STATE){
    //    return false;
    //}

    GPIO_setOutputHighOnPin(
        GPIO_PORT_P1,
        GPIO_PIN1
        );
    valve.state = VALVE_OPENING_STATE;

    valveControl_delay();
    return true;
}

void valveControl_delay(){
    if(valve.command_Pulse_Delay.state == UT_TMR_DELAY_INIT){
        valve.command_Pulse_Delay.state = UT_TMR_DELAY_WAIT;
        hal_timer_a_InitValveDelay();
    }
}

bool valveControl_close()
{
    /*
     EL PIN ES EL PRIMER N�MERO DE LA SIGUIENTE TABLA
     CONECTOR J10 DEL EVM:

     1: Tx5VEn  -> P1.1
     2: XPB0    -> P7.0
     3: RxPwr   -> P2.2
     4: RxEn    -> P6.7
     5: TxSel   -> P2.3
     6: RxSel1  -> P3.6
     7: TxPwr   -> P1.0
     8: RxSel2  -> P3.7
    */
    //if (valve.state != VALVE_OPEN_STATE){
    //    return false;
    //}

    GPIO_setOutputHighOnPin(
        GPIO_PORT_P7,
        GPIO_PIN0
        );
    valve.state = VALVE_CLOSING_STATE;

    valveControl_delay();
    return true;

}

void valveControl_update ()
{
    switch(valve.state)
    {
    case VALVE_CLOSING_STATE:
        if(UT_timer_delay(&valve.command_Pulse_Delay)){
            GPIO_setOutputLowOnPin(
                GPIO_PORT_P7,
                GPIO_PIN0
                );
            Communications_clearBusy();
            valve.state = VALVE_CLOSED_STATE;

        }
        break;
     case VALVE_OPENING_STATE:
         if(UT_timer_delay(&valve.command_Pulse_Delay)){
             GPIO_setOutputLowOnPin(
                 GPIO_PORT_P1,
                 GPIO_PIN1
                 );
             Communications_clearBusy();
             valve.state = VALVE_OPEN_STATE;
         }
         break;

     /* This code is just an extra safety measure in case for some reason the state of
     * the valve is undefined, however, if that happens the code will crash. So this
     * extra step isn't needed.
     *
     case VALVE_OPEN_STATE: break;
     case VALVE_CLOSED_STATE: break;
     default:
         // Should never happen
         valve.state = VALVE_CLOSED_STATE;
         valveControl_close();
         break;
     */
    }

}
