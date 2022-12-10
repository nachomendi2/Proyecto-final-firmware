/*
 * ValveControl.c
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */

#include "ValveControl.h"
#include <hal_timer_a.h>

ValveControl_Module valve;

// DEPRECATED:
bool ValveReceiveOrder () // Por ahora los tres botones dan la misma orden
{
    if (GPIO_getInputPinValue(GPIO_PORT_PJ, GPIO_PIN0))
    {
        return false;
    } else
    {
        return true;
    }
}

void valveControl_setup(ValveState initial_state)
{
    //initial state should be loaded from FRAM, by default assume it's closed
    if (initial_state == VALVE_INIT_STATE){
        valve.state = VALVE_CLOSED_STATE;
        valveControl_close();
    }else{
        valve.state = initial_state;
    }

    //Seteo los puertos expuestos del GPIO del EVM, y sus pines. (OUTPUT). Comienzan todas en LOW.
    GPIO_setAsOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN3
        );

    GPIO_setAsOutputPin(
        GPIO_PORT_P7,
        GPIO_PIN0
        );


    //Seteamos en LOW
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
    if (valve.state != VALVE_CLOSED_STATE){
        return false;
    }

    GPIO_setOutputHighOnPin(
        GPIO_PORT_P1,
        GPIO_PIN1
        );
    valve.state = VALVE_OPENING_STATE;

    valveControl_delay();
    return true;
}

void valveControl_delay(){
    valve.command_pulse_delay = UT_TMR_DELAY_WAIT;
    hal_timer_a_InitValveDelay();
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
    if (valve.state != VALVE_OPEN_STATE){
        return false;
    }

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
        if(UT_timer_delay(valve.command_pulse_delay)){
            GPIO_setOutputLowOnPin(
                GPIO_PORT_P7,
                GPIO_PIN0
                );
            valve.state = VALVE_CLOSED_STATE;

        }
        break;
     case VALVE_OPENING_STATE:
         if(UT_timer_delay(&valve.command_pulse_delay)){
             GPIO_setOutputLowOnPin(
                 GPIO_PORT_P1,
                 GPIO_PIN1
                 );
             valve.state = VALVE_OPEN_STATE;
         }
         break;
     default:
         // Should never happen
         valve.state = VALVE_CLOSED_STATE;
         valveControl_close();
         break;
    }

}
