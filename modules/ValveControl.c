/*
 * ValveControl.c
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */

#include "ValveControl.h"

ValveState NextState = VALVE_INIT_STATE;

bool ValveReceiveOrder () //Por ahora los tres botones dan la misma orden
{
    if (GPIO_getInputPinValue(GPIO_PORT_PJ, GPIO_PIN0))
    {
        return false;
    } else
    {
        return true;
    }
}

void ValveConfig ()
{
    //Seteo los puertos expuestos del GPIO del EVM, y sus pines. (OUTPUT). Comienzan todas en LOW.
    GPIO_setAsOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN1
        );

    GPIO_setAsOutputPin(
        GPIO_PORT_P7,
        GPIO_PIN0
        );


    //Seteamos en LOW
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P1,
            GPIO_PIN1
            );

    GPIO_setOutputLowOnPin(
            GPIO_PORT_P7,
            GPIO_PIN0
            );

}

void ToggleValve (uint8_t SelectedPin)
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

bool OpenValve ()
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

    GPIO_setOutputHighOnPin(
        GPIO_PORT_P1,
        GPIO_PIN1
        );

    __delay_cycles(10000000);

    GPIO_setOutputLowOnPin(
        GPIO_PORT_P1,
        GPIO_PIN1
        );

    return true;
}

bool CloseValve ()
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

    GPIO_setOutputHighOnPin(
        GPIO_PORT_P7,
        GPIO_PIN0
        );

    __delay_cycles(10000000);

    GPIO_setOutputLowOnPin(
        GPIO_PORT_P7,
        GPIO_PIN0
        );

    return true;

}

bool SM_Valve ()
{
    while(1)
    {
        switch(NextState)
        {

        case VALVE_INIT_STATE:

            ValveConfig();

            if (CloseValve())
            {
                NextState = VALVE_CLOSED_STATE;  //Returns true as the ValveSM is not busy
                return true;
            } else
            {
                NextState = VALVE_CLOSING_STATE; //Returns false as the ValveSM is busy //Por ahora nunca va a llegar a esto, hasta mejorar el delay
                return false;
            }


        case VALVE_CLOSED_STATE:

            if (ValveReceiveOrder()) //Crear funcion ReceiveOrder
            {
                if (OpenValve())
                {
                   NextState = VALVE_OPEN_STATE;
                   return true;
                } else
                {
                    NextState = VALVE_OPENING_STATE; //Nunca va a llegar a esto por ahroa
                    return false;
                }
            } else
            {
                NextState = VALVE_CLOSED_STATE;
                return true;
            }


         case VALVE_OPEN_STATE:

             if (ValveReceiveOrder())
             {
                 if (CloseValve())
                 {
                     NextState = VALVE_CLOSED_STATE;
                     return true;
                 } else
                 {
                     NextState = VALVE_CLOSING_STATE;
                     return false;
                 }
             } else
             {
                 NextState = VALVE_OPEN_STATE;
                 return true;
             }


         default:
             NextState = VALVE_INIT_STATE;  //Nunca deberia llegar a esto
             return false;
        }
    }
}
