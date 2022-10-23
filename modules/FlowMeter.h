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

USS_message_code flowMeter_setup();

_iq16 flowMeter_getVolumeFlowRate();

#endif /* MODULES_FLOWMETER_H_ */
