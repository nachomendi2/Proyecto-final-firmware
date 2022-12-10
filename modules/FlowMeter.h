/*
 * FlowMeasure.h
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */

#ifndef MODULES_FLOWMETER_H_
#define MODULES_FLOWMETER_H_

#include "ussSwLib.h"
#include "USS_userConfig.h"
#include "USS_App_userConfig.h"
#include <driverlib.h>
#include <hal.h>
#include <IQmathlib.h>
#include <stdbool.h>

// Define constants For calculating Mass flow rate (use #define instead of const to save memory):
#define ATMOSPHERIC_PRESSURE 65536000 // 1000 mbar expressed in fixed point
#define LPG_REFERENCE_DENSITY 124387 // expressed in kg/m^3
#define MASS_FLOW_RATE_CALCULATION_CONST_1 18884198// 288.15 expressed in fixed point
#define MASS_FLOW_RATE_CALCULATION_CONST_2 17901158// 273.15 expressed in fixed point
#define MASS_FLOW_RATE_CALCULATION_CONST_3 66404352// 1013.25 expressed in fixed point

USS_message_code flowMeter_setup();

_iq16 flowMeter_getVolumeFlowRate();

_iq16 flowMeter_getTemperature();

_iq16 flowMeter_getDensity();

_iq16 flowMeter_getPressure();

_iq16 flowMeter_getMassFlowRate(_iq16 vol_flow_rate);

void flowMeter_measure();

#endif /* MODULES_FLOWMETER_H_ */
