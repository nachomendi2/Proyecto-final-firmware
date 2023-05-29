/*
 * PressureSensor.c
 *
 *  Created on: 7 ene. 2023
 *      Author: ignaciomendizabal
 */

#include "PressureSensor.h"
#include "eusci_b_i2c.h"
#include "gpio.h"
#include "stdbool.h"
#include "cs.h"
#include "Communications.h"
#include <IQmathlib.h>

PressureSensor_Module pressure_sensor = {NULL,NULL,NULL,NULL};

void PressureSensor_setup(){

    WDT_A_hold(WDT_A_BASE);

    pressure_sensor.status = PRESSURE_SENSOR_STATUS_INACTIVE;

    GPIO_setOutputHighOnPin(
        GPIO_PORT_P1,
        GPIO_PIN0
        );

    PMM_unlockLPM5();

    // 2. Init eUSCI_B module configuration
    EUSCI_B_I2C_initMasterParam param = {0};
    param.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
    param.i2cClk = CS_getSMCLK();
    param.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
    param.byteCounterThreshold = 1;
    param.autoSTOPGeneration = EUSCI_B_I2C_SET_BYTECOUNT_THRESHOLD_FLAG;
    EUSCI_B_I2C_initMaster(EUSCI_B0_BASE, &param);

   // 3. Set pressure sensor address (specified on datasheet)
   EUSCI_B_I2C_setSlaveAddress(EUSCI_B0_BASE,
           PRESSURE_SENSOR_I2C_SLAVE_ADDRESS
           );

   EUSCI_B_I2C_setMode(
                   EUSCI_B0_BASE,
                   EUSCI_B_I2C_TRANSMIT_MODE
                   );

   // 4. Start eUSCI_B1 module
   EUSCI_B_I2C_enable(EUSCI_B0_BASE);


   EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE,
                   EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
                   EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
                   EUSCI_B_I2C_NAK_INTERRUPT
                 );

}

uint8_t PressureSensor_readRegister(uint8_t sensor_register)
{
    // Transmit to sensor which register to read
    EUSCI_B_I2C_setMode(
                EUSCI_B0_BASE,
                EUSCI_B_I2C_TRANSMIT_MODE
                );

    EUSCI_B_I2C_masterSendSingleByte(EUSCI_B0_BASE, sensor_register);
    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE,
                    EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
                    EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
                    EUSCI_B_I2C_NAK_INTERRUPT
                  );
    EUSCI_B_I2C_setMode(EUSCI_B0_BASE,
            EUSCI_B_I2C_RECEIVE_MODE
            );


    // Wait for SDA & SCL lines to update, meanwhile update Communications module:
    uint16_t i = 100; //Set to smallest possible value that doesn't break I2C communication
    while (i--)
    {
        Communications_update();
    }

    // Another alternative is to use a delay, this makes measurement times more consistent, although
    // it blocks the CPU from checking for new frames on the Communications module, which may
    // result in frame loss which is less preferable than slower measurement times.
    //__delay_cycles(2000);

    // Read selected register & return result:

    uint8_t response = EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B0_BASE);
    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE,
                    EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
                    EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
                    EUSCI_B_I2C_NAK_INTERRUPT
                  );
    return response;
}

bool PressureSensor_writeRegister(uint8_t sensor_register, uint8_t write_data)
{
    EUSCI_B_I2C_setMode(
                    EUSCI_B0_BASE,
                    EUSCI_B_I2C_TRANSMIT_MODE
                    );

    // Transmit to sensor which register to write:
    EUSCI_B_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, sensor_register);

    // Transmit to sensor which value to write to selected register:
    EUSCI_B_I2C_masterSendMultiByteFinish(EUSCI_B0_BASE, write_data);

    // Clear Interrupt flags (flags trigger although interrupts are disabled)
    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE,
                    EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
                    EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
                    EUSCI_B_I2C_BYTE_COUNTER_INTERRUPT +
                    EUSCI_B_I2C_NAK_INTERRUPT
                  );
    return true;
}

void PressureSensor_update(){

    // Turn ON pressure sensor:
    GPIO_setOutputHighOnPin(
        GPIO_PORT_P1,
        GPIO_PIN0
        );
    __delay_cycles(5000);

    // 1. Send 'start measurement' order to pressure sensor:
    PressureSensor_writeRegister(
            PRESSURE_SENSOR_I2C_CMD_REGISTER,
            PRESSURE_SENSOR_I2C_COMMAND_START_MEASUREMENT
            );

    // 2. Wait until measurement is complete by reading 'CMD' register:
    uint8_t status = PressureSensor_readRegister(PRESSURE_SENSOR_I2C_CMD_REGISTER);
    while(~status & PRESSURE_SENSOR_I2C_MEASUREMENT_COMPLETE != PRESSURE_SENSOR_I2C_MEASUREMENT_COMPLETE){

        // We can continue updating the Communications module while we wait in case new packages arrive.
        // WARNING: This could slow down the measurement process if a big frame is requested (e.g: status)
        Communications_update();
        status = PressureSensor_readRegister(PRESSURE_SENSOR_I2C_CMD_REGISTER);
    }

    // 3. Read measurement results & update pressure & temperature values.
    uint32_t pressure_byte1 = PressureSensor_readRegister(0x06);
    //Communications_update();
    uint32_t pressure_byte2 = PressureSensor_readRegister(0x07);
    //Communications_update();
    uint32_t pressure_byte3 = PressureSensor_readRegister(0x08);
    //Communications_update();
    uint16_t temperature_byte1 = PressureSensor_readRegister(0x09);
    //Communications_update();
    uint16_t temperature_byte2 = PressureSensor_readRegister(0x0A);
    //Communications_update();

    // Turn OFF pressure sensor:
    GPIO_setOutputLowOnPin(
        GPIO_PORT_P1,
        GPIO_PIN0
        );

    // TODO: Process temp & pressure bytes & store them on `pressure_sensor` struct with according units!
    uint32_t Pressure;
    uint32_t Temperature;
    uint32_t K = 8192;
    uint32_t pressure_adc;


    float press;
    uint8_t NegativePressure = 0;
    uint8_t NegativeTemperature = 0;
    NegativePressure = (0b100000000|pressure_byte1); //Check if negative pressure
    NegativeTemperature = (0b10000000|temperature_byte1);

    __no_operation();
    Temperature = temperature_byte2;
    Temperature |=temperature_byte1<<8;

    if (NegativeTemperature) {
        Temperature = _IQdiv16(_IQdiv16((Temperature-2^16)));
    } else {
        Temperature = _IQdiv16(_IQdiv16(Temperature));
    }
    pressure_sensor.temperature = Temperature;


    Pressure = pressure_byte1 << 16;
    Pressure |= pressure_byte2 << 8;
    Pressure |= pressure_byte3;

    if (Pressure > 8388608) {
        Pressure = (Pressure - 16777216) / K;
    } else {
        Pressure = Pressure / K;
    }

    pressure_sensor.pressure = Pressure; //In Pa -- 1 Pa = 0.01 mbar
    __no_operation();
}

inline _iq16 PressureSensor_getTemperatureFixedPoint(){
    return pressure_sensor.temperature;
}

inline _iq16 PressureSensor_getPressureFixedPoint(){
    return pressure_sensor.pressure;
}

inline uint32_t PressureSensor_getTemperature(){
    return _IQ16int (pressure_sensor.temperature);
}

inline uint32_t PressureSensor_getPressure(){
    return pressure_sensor.pressure;
}
