/*
 * PressureSensor.h
 *
 *  Created on: 7 ene. 2023
 *      Author: ignaciomendizabal
 */

#ifndef MODULES_PRESSURESENSOR_H_
#define MODULES_PRESSURESENSOR_H_

#include <IQmathlib.h>

typedef struct{
    _iq16 temperature;
    _iq16 pressure;
}PressureSensor_Module;


#endif /* MODULES_PRESSURESENSOR_H_ */
