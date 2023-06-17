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
#define VSF 14841283 // 226.46 expressed in fixed point
#define FIXED_POINT_BITS    16
#define SCALING_FACTOR      (1 << FIXED_POINT_BITS)
#define MINIMUM_FLUX        50
#define MAX_SUPPORTED_DToF  13f
#define MIN_SUPPORTED_DToF  1.5f
#define MEASURE_WAKE_UP_TIME 1


typedef struct
{
    float totalizer;
    uint16_t measurement_Count;
    _iq16 last_Volume_Flow_Rate;
    _iq16 last_Mass_Flow_Rate;
    uint16_t measure_Time_Interval_Seconds;
    _iq16 last_DToF;
}flowMeter_Module;


USS_message_code flowMeter_setup();

_iq16 flowMeter_measureDToF();

_iq16 flowMeter_calculateVolumeFlowRate(_iq16 DToF);

inline _iq16 flowMeter_getDensity();

float flowMeter_calculateMassFlowRate(_iq16 DToF);

void flowMeter_update();

inline float flowMeter_getAverageMassFlowRate();

inline float flowMeter_getVolumeFlowRate();

inline float flowMeter_getTotalizer();

inline float flowMeter_getMassFlowRate();

void flowMeter_reset();

#endif /* MODULES_FLOWMETER_H_ */
