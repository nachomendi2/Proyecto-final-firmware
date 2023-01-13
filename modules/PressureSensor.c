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

PressureSensor_Module pressure_sensor;

void PressureSensor_setup(){
    WDT_A_hold(WDT_A_BASE);
    pressure_sensor.status = PRESSURE_SENSOR_STATUS_INACTIVE;

    // 1. Configure pzins 6 & 7 of port 3 to use the eUSCI_B module instead of GPIO
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P3,
        GPIO_PIN6 + GPIO_PIN7,
        GPIO_PRIMARY_MODULE_FUNCTION
        );
    PMM_unlockLPM5();

    // 2. Init eUSCI_B module configuration
    EUSCI_B_I2C_initMasterParam param = {0};
    param.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
    param.i2cClk = CS_getSMCLK();
    param.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
    param.byteCounterThreshold = 0;
    param.autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP;
   EUSCI_B_I2C_initMaster(EUSCI_B1_BASE, &param);

   // 3. Set pressure sensor address (specified on datasheet)
   EUSCI_B_I2C_setSlaveAddress(EUSCI_B1_BASE,
           PRESSURE_SENSOR_I2C_SLAVE_ADDRESS
           );

   EUSCI_B_I2C_setMode(
                   EUSCI_B1_BASE,
                   EUSCI_B_I2C_TRANSMIT_MODE
                   );

   // 4. Start eUSCI_B1 module
   EUSCI_B_I2C_enable(EUSCI_B1_BASE);

   // 5. Enable interrupts
   EUSCI_B_I2C_clearInterrupt(EUSCI_B1_BASE,
                      EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
                      EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
                      EUSCI_B_I2C_NAK_INTERRUPT
                    );

   EUSCI_B_I2C_enableInterrupt(EUSCI_B1_BASE,
                   EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
                   EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
                   EUSCI_B_I2C_NAK_INTERRUPT
                 );

}

uint8_t PressureSensor_readRegister(uint8_t sensor_register)
{

    EUSCI_B_I2C_setMode(
                EUSCI_B1_BASE,
                EUSCI_B_I2C_TRANSMIT_MODE
                );

    //EUSCI_B_I2C_masterSendStart(EUSCI_B1_BASE);
    EUSCI_B_I2C_masterSendMultiByteStart(EUSCI_B1_BASE, sensor_register);
    PressureSensor_waitForByteTransmissionComplete();
    EUSCI_B_I2C_setMode(
                    EUSCI_B1_BASE,
                    EUSCI_B_I2C_RECEIVE_MODE
                    );

    EUSCI_B_I2C_masterSendStart(EUSCI_B1_BASE);
    PressureSensor_waitForByteReceptionComplete();
    uint8_t response = EUSCI_B_I2C_masterReceiveSingle(EUSCI_B1_BASE);// EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);//EUSCI_B_I2C_masterReceiveMultiByteFinish(EUSCI_B1_BASE);
    EUSCI_B_I2C_masterReceiveMultiByteStop(EUSCI_B1_BASE);
    return response;
}

bool PressureSensor_writeRegister(uint8_t sensor_register, uint8_t write_data)
{
    EUSCI_B_I2C_setMode(
                    EUSCI_B1_BASE,
                    EUSCI_B_I2C_TRANSMIT_MODE
                    );

    EUSCI_B_I2C_masterSendMultiByteStart(EUSCI_B1_BASE, sensor_register);
    PressureSensor_waitForByteTransmissionComplete();
    EUSCI_B_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, write_data);
    PressureSensor_waitForByteTransmissionComplete();
    EUSCI_B_I2C_masterSendMultiByteStop(EUSCI_B1_BASE);
    return true;
}

inline void PressureSensor_waitForByteTransmissionComplete(){
    while(!pressure_sensor.byte_Tx_ready)
    {
        Communications_update();
    }
    pressure_sensor.byte_Tx_ready = false;
}


inline void PressureSensor_waitForByteReceptionComplete(){
    while(!pressure_sensor.byte_Rx_received)
    {
        Communications_update();
    }
    pressure_sensor.byte_Rx_received = false;
}

uint8_t aa,bb,cc,dd,ee;
void PressureSensor_update(){

    __no_operation();

    // 1. Send 'start measurement' order to pressure sensor:
    PressureSensor_writeRegister(
            PRESSURE_SENSOR_I2C_CMD_REGISTER,
            PRESSURE_SENSOR_I2C_COMMAND_START_MEASUREMENT
            );

    // 2. Wait until measurement is complete by reading 'CMD' register:

    uint8_t status = PressureSensor_readRegister(PRESSURE_SENSOR_I2C_CMD_REGISTER);
    while(~status & PRESSURE_SENSOR_I2C_MEASUREMENT_COMPLETE != PRESSURE_SENSOR_I2C_MEASUREMENT_COMPLETE){
        //Communications_update();
        status = PressureSensor_readRegister(PRESSURE_SENSOR_I2C_CMD_REGISTER);
    }

    // 3. Read measurement results & update pressure & temperature values.
    uint8_t pressure_byte1 = PressureSensor_readRegister(0x06);
    uint8_t pressure_byte2 = PressureSensor_readRegister(0x07);
    uint8_t pressure_byte3 = PressureSensor_readRegister(0x08);
    uint8_t temperature_byte1 = PressureSensor_readRegister(0x09);
    uint8_t temperature_byte2 = PressureSensor_readRegister(0x0A);
    pressure_sensor.temperature = 0;
    pressure_sensor.temperature = temperature_byte1;
    pressure_sensor.temperature |= (uint16_t) temperature_byte2 << 8;
    aa = pressure_byte1;
    bb = pressure_byte2;
    cc = pressure_byte3;
    dd = temperature_byte1;
    ee = temperature_byte2;
    __no_operation();

}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B1_VECTOR
__interrupt void USCI_B1_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(EUSCI_B1_VECTOR))) USCI_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{

    if(EUSCI_B_I2C_getInterruptStatus(
                EUSCI_B1_BASE,
                EUSCI_B_I2C_NAK_INTERRUPT) == EUSCI_B_I2C_NAK_INTERRUPT){
        EUSCI_B_I2C_clearInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_NAK_INTERRUPT);
    }

    if(EUSCI_B_I2C_getInterruptStatus(
            EUSCI_B1_BASE,
            EUSCI_B_I2C_TRANSMIT_INTERRUPT0) == EUSCI_B_I2C_TRANSMIT_INTERRUPT0){
        pressure_sensor.byte_Tx_ready = true;
        EUSCI_B_I2C_clearInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
        return;
    }

    if(EUSCI_B_I2C_getInterruptStatus(
             EUSCI_B1_BASE,
             EUSCI_B_I2C_RECEIVE_INTERRUPT0) == EUSCI_B_I2C_RECEIVE_INTERRUPT0){
        EUSCI_B_I2C_clearInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);
        pressure_sensor.byte_Rx_received = true;
        return;
    }
}
