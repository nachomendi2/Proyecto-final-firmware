/*
 * PressureSensor.h
 *
 *  Created on: 7 ene. 2023
 *      Author: ignaciomendizabal
 */

#ifndef MODULES_PRESSURESENSOR_H_
#define MODULES_PRESSURESENSOR_H_

#include <IQmathlib.h>
#include <stdbool.h>

#define PRESSURE_SENSOR_I2C_SLAVE_ADDRESS 0x6D

#define PRESSURE_SENSOR_I2C_COMMAND_START_MEASUREMENT 0x0A
#define PRESSURE_SENSOR_I2C_CMD_REGISTER 0x30
#define PRESSURE_SENSOR_I2C_MEASUREMENT_COMPLETE 0x08

typedef enum{
    PRESSURE_SENSOR_STATUS_INACTIVE,
    PRESSURE_SENSOR_STATUS_START_MEASUREMENT,
    PRESSURE_SENSOR_STATUS_MEASURING,
    PRESSURE_SENSOR_STATUS_READ_RESULTS_BYTE1,
    PRESSURE_SENSOR_STATUS_READ_RESULTS_BYTE2,
    PRESSURE_SENSOR_STATUS_READ_RESULTS_BYTE3,
    PRESSURE_SENSOR_STATUS_READ_RESULTS_BYTE4,
    PRESSURE_SENSOR_STATUS_READ_RESULTS_BYTE5,
}PressureSensor_status;

typedef struct{
    _iq16 temperature;
    _iq16 pressure;
    bool byte_Tx_ready;
    bool byte_Rx_received;
    PressureSensor_status status;
}PressureSensor_Module;

void PressureSensor_setup();

uint8_t PressureSensor_readRegister(uint8_t sensor_register);

bool PressureSensor_writeRegister(uint8_t sensor_register, uint8_t write_data);

void PressureSensor_update();

inline _iq16 PressureSensor_getTemperatureFixedPoint();

inline _iq16 PressureSensor_getPressureFixedPoint();

inline uint32_t PressureSensor_getTemperature();

inline uint32_t PressureSensor_getPressure();


#endif /* MODULES_PRESSURESENSOR_H_ */
