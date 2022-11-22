/*
 * ValveControl.h
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */

#ifndef MODULES_VALVECONTROL_H_
#define MODULES_VALVECONTROL_H_

#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"

typedef enum
{
    VALVE_INIT_STATE,
    VALVE_CLOSED_STATE,
    VALVE_CLOSING_STATE,
    VALVE_OPEN_STATE,
    VALVE_OPENING_STATE

}ValveState;

bool SM_Valve();

bool ValveReceiveOrder();

void ValveConfig ();

void ToggleValve (uint8_t SelectedPin);

bool OpenValve();

bool CloseValve();

#endif /* MODULES_VALVECONTROL_H_ */
