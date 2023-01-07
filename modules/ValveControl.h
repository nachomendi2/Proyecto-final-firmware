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
#include <utils.h>

typedef enum
{
    VALVE_INIT_STATE = 0,
    VALVE_CLOSED_STATE = 1,
    VALVE_CLOSING_STATE = 2,
    VALVE_OPEN_STATE = 3,
    VALVE_OPENING_STATE = 4

}ValveState_t;

typedef struct
{
    ValveState_t state;
    ut_tmrDelay_t command_Pulse_Delay;
}ValveControl_Module;

ValveState_t valveControl_getValveState();

void valveControl_update();

bool valveControl_close();

void valveControl_delay();

bool valveControl_open();

void valveControl_toggle(uint8_t SelectedPin);

void valveControl_setup();

#endif /* MODULES_VALVECONTROL_H_ */
