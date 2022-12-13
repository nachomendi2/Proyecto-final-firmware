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
    VALVE_INIT_STATE,
    VALVE_CLOSED_STATE,
    VALVE_CLOSING_STATE,
    VALVE_OPEN_STATE,
    VALVE_OPENING_STATE

}ValveState_t;

typedef struct
{
    ValveState_t state;
    ut_tmrDelay_t command_pulse_delay;
}ValveControl_Module;

void valveControl_update();

bool valveControl_close();

void valveControl_delay();

bool valveControl_open();

void valveControl_toggle(uint8_t SelectedPin);

void valveControl_setup(ValveState_t initial_state);

#endif /* MODULES_VALVECONTROL_H_ */
