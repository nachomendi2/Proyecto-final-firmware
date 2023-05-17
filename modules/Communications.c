/*
 * Communications.c
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */
#include <msp430.h>
#include <eusci_a_spi.h>
#include <gpio.h>
#include <Communications.h>
#include <stdbool.h>
#include <stdint.h>
#include <IQmathlib.h>
#include "utils.h"
#include <ValveControl.h>
#include "flowMeter.h"
#include "PressureSensor.h"

SPI_Communications_Module SPI_slave = {0};
uint16_t received_byte;

// Initial setup of Communications module
void Communications_setup(void){

    SPI_slave.communication_Status = COMMUNICATION_STATUS_INACTIVE;
    WDT_A_hold(WDT_A_BASE);

    /* 1. Configure pins for eUSCI_A2:
     * Change Peripheral Module Function of pins 0,1,2,3,4,5 of PORT J to use eUSCI_A2 instead
     *  - Set PIN 0 as Input (CLK)
     *  - Set PIN 1 as Input (STE) (Unused)
     *  - Set PIN 2 as Input (MOSI)
     *  - Set PIN 3 as Output (MISO)
     */

    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN1);

    GPIO_setAsInputPin(GPIO_PORT_P5, GPIO_PIN0 + GPIO_PIN2 + GPIO_PIN3);


    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P5,
        GPIO_PIN0 + GPIO_PIN2 + GPIO_PIN3,
        GPIO_SECONDARY_MODULE_FUNCTION
        );

    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P5,
        GPIO_PIN1,
        GPIO_SECONDARY_MODULE_FUNCTION
        );

    //2. Configure GPIO for slave select: EVM= P2.3, PCB= P5.4
    //GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5,
    //                                     GPIO_PIN4
    //                                     );
    GPIO_setAsInputPin(
        GPIO_PORT_P5,
        GPIO_PIN4
        );

    GPIO_enableInterrupt(
        GPIO_PORT_P5,
        GPIO_PIN4
        );

    GPIO_selectInterruptEdge(
        GPIO_PORT_P5,
        GPIO_PIN4,
        GPIO_HIGH_TO_LOW_TRANSITION
        );

    /*
     * 3. Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    // 4. Initialize eUSCI module in SPI slave mode:
    EUSCI_A_SPI_initSlaveParam params = {0};

    // The following parameters are set by default:

    params.msbFirst = EUSCI_A_SPI_MSB_FIRST;
    params.clockPhase = EUSCI_A_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
    params.clockPolarity = EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    params.spiMode = EUSCI_A_SPI_3PIN;

    EUSCI_A_SPI_initSlave(EUSCI_A2_BASE, &params);

    // 5. Start eUSCI_A2 module:
    EUSCI_A_SPI_enable(EUSCI_A2_BASE);

    // Aditional: 6. configure busy pin (EVM = P6.7, PCB = P5.5)
    GPIO_setAsOutputPin(
        GPIO_PORT_P5,
        GPIO_PIN5
        );
}

// Sends a frame via SPI, return false if data can't be send, or true if data was transmitted successfully
bool Communications_send(SPI_Communications_Frame frame){

    // Just as safety measure, make sure frame length is less than buffer size:
    if(frame.frame_Length > SPI_TX_MAX_BUFFER_SIZE){
        return false;
    }

    SPI_slave.byte_Transmit_Counter = 0;
    SPI_slave.transmit_Frame_length = frame.frame_Length;
    SPI_slave.byte_Transmit_buffer[0] = 42; // ASCII for *
    SPI_slave.byte_Transmit_buffer[1] = frame.frame_Type;
    SPI_slave.byte_Transmit_buffer[2] = frame.frame_Length;

    uint8_t i;
    for(i=0;i<(frame.frame_Length-4); i++){ // substract head, type, length & CRC from frame length
        SPI_slave.byte_Transmit_buffer[i+3] = *(frame.frame_Body + i);
    }

    SPI_slave.byte_Transmit_buffer[frame.frame_Length - 1] = Communications_CRC8(SPI_slave.byte_Transmit_buffer, frame.frame_Length - 1);

    // Enable transmit interrupts
    EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);

    EUSCI_A_SPI_enableInterrupt(
                               EUSCI_A2_BASE,
                               EUSCI_A_SPI_TRANSMIT_INTERRUPT
                               );

    return true;
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = PORT5_VECTOR
__interrupt void Port5_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT5_VECTOR))) Port5_ISR (void)
#else
#error Compiler not supported!
#endif
{
    GPIO_clearInterrupt(GPIO_PORT_P5,GPIO_PIN4); //PCB
       switch(GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN4)){ //PCB
           case GPIO_INPUT_PIN_LOW:
               SPI_slave.communication_Status = COMMUNICATION_STATUS_LISTENING;
               EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
               EUSCI_A_SPI_enableInterrupt(
                                          EUSCI_A2_BASE,
                                          EUSCI_A_SPI_RECEIVE_INTERRUPT
                                          );
               GPIO_selectInterruptEdge( //PCB
                       GPIO_PORT_P5,
                       GPIO_PIN4,
                       GPIO_LOW_TO_HIGH_TRANSITION
                       );
               break;
           case GPIO_INPUT_PIN_HIGH:

               SPI_slave.communication_Status = COMMUNICATION_STATUS_INACTIVE;
               EUSCI_A_SPI_disableInterrupt(
                       EUSCI_A2_BASE,
                       EUSCI_A_SPI_RECEIVE_INTERRUPT
                       );
               GPIO_selectInterruptEdge( //PCB
                       GPIO_PORT_P5,
                       GPIO_PIN4,
                       GPIO_HIGH_TO_LOW_TRANSITION
                       );
               break;
       }
   __bic_SR_register_on_exit(CPUOFF); // wake up from LPM
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A2_VECTOR
__interrupt void USCI_A2_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(EUSCI_A2_VECTOR))) USCI_A2_ISR (void)
#else
#error Compiler not supported!
#endif
{
    /* Sets Tx & Rx flags from SPI_slave module to indicate whether the code should
     * process the next byte to send (Tx flag ON) or read the next byte (Rx ON) on
     * Communications_update() function
     */

    // If transmit interrupt raised, set Tx_ready flag
    if (EUSCI_A_SPI_getInterruptStatus(
            EUSCI_A2_BASE,
            EUSCI_A_SPI_TRANSMIT_INTERRUPT) == EUSCI_A_SPI_TRANSMIT_INTERRUPT) {
        EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE,EUSCI_A_SPI_TRANSMIT_INTERRUPT);
        SPI_slave.byte_Tx_ready = true;
        return;
    }

    // If receive interrupt raised, set Rx_received flag
    if (EUSCI_A_SPI_getInterruptStatus(
                EUSCI_A2_BASE,
                EUSCI_A_SPI_RECEIVE_INTERRUPT) == EUSCI_A_SPI_RECEIVE_INTERRUPT) {
        EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE,EUSCI_A_SPI_RECEIVE_INTERRUPT);
        received_byte = EUSCI_A_SPI_receiveData(EUSCI_A2_BASE);
        SPI_slave.byte_Rx_received = true;
        return;
    }
    __bic_SR_register_on_exit(CPUOFF); // wake up from LPM

}

// Returns whether the communications module is working
bool Communications_isActive(){
    if (SPI_slave.communication_Status != COMMUNICATION_STATUS_INACTIVE) {
        return true;
    }else{
        return false;
    }
}

// Processes a given frame & sends the corresponding response via SPI
void Communications_ProcessRequest(SPI_Communications_Frame request){


    SPI_Communications_Frame response;
    ValveState_t valve_state;

    // define auxiliary union variable to convert from float to an array of bytes:
    union {
        float float_value;
        uint8_t byte_array[4];
    }aux_float2bytes;

    union {
        uint32_t uint32_value;
        uint8_t byte_array[4];
    }aux_uint2bytes;

    switch(request.frame_Type){
    case FRAME_REQUEST_STATUS:

        uint8_t status_body[29];
        status_body[0] = valveControl_getValveState();
        aux_float2bytes.float_value = flowMeter_getVolumeFlowRate();
        for (uint8_t i=1; i<5;i++){
            status_body[i] = aux_float2bytes.byte_array[4-i];
        }
        aux_float2bytes.float_value = PressureSensor_getPressure();
        for (uint8_t i=5; i<9;i++){
            status_body[i] = aux_float2bytes.byte_array[8-i];
        }
        aux_float2bytes.float_value = PressureSensor_getTemperature();
        for (uint8_t i=9; i<13;i++){
            status_body[i] = aux_float2bytes.byte_array[12-i];
        }
        aux_float2bytes.float_value = flowMeter_getMassFlowRate();
        for (uint8_t i=13; i<17;i++){
            status_body[i] = aux_float2bytes.byte_array[16-i];
        }
        // TODO: measure battery
        for (uint8_t i=17; i<21;i++){
            status_body[i] = 0xAAAAAAAA;
        }
        //aux_float2bytes.float_value = flowMeter_getTotalizer();
        aux_uint2bytes.uint32_value = (int)flowMeter_getTotalizer();
        //int flujo_test = (int)flowMeter_getTotalizer();
        for (uint8_t i=21; i<25;i++){
            //status_body[i] = aux_float2bytes.byte_array[24-i];
            status_body[i] = aux_uint2bytes.byte_array[24-i];
        }
        aux_float2bytes.float_value = flowMeter_getAverageMassFlowRate();
        for (uint8_t i=25; i<29;i++){
            status_body[i] = aux_float2bytes.byte_array[28-i];
        }

        response.frame_Body = status_body;
        response.frame_Type = FRAME_RESPONSE_STATUS;
        response.frame_Length = 33;
        break;

    case FRAME_REQUEST_AVERAGE_MASS_FLOW_RATE:

        // Get flow rate as float and convert to byte array using auxiliary union variable:
        aux_float2bytes.float_value = flowMeter_getAverageMassFlowRate();
        response.frame_Body = aux_float2bytes.byte_array;
        response.frame_Type = FRAME_RESPONSE_AVERAGE_MASS_FLOW_RATE;
        response.frame_Length = 8;
        break;

    case FRAME_REQUEST_TOTALIZER:

        // Get flow rate as float and convert to byte array using auxiliary union variable:
        aux_float2bytes.float_value = flowMeter_getTotalizer();
        response.frame_Body = aux_float2bytes.byte_array;
        response.frame_Type = FRAME_RESPONSE_TOTALIZER;
        response.frame_Length = 8;
        break;

    case FRAME_REQUEST_OPEN_VALVE:

        // Set busy pin & set frame body to the current state of the valve:
        valveControl_open();
        valve_state = valveControl_getValveState();
        Communications_setBusy();
        response.frame_Body = &valve_state;
        response.frame_Type = FRAME_RESPONSE_VALVE_ACK;
        response.frame_Length = 5;

        break;

    case FRAME_REQUEST_CLOSE_VALVE:

        // Set busy pin & set frame body to the current state of the valve:
        valveControl_close();
        valve_state = valveControl_getValveState();
        Communications_setBusy();
        response.frame_Body = &valve_state;
        response.frame_Type = FRAME_RESPONSE_VALVE_ACK;
        response.frame_Length = 5;

        break;
    case FRAME_REQUEST_CONFIGURE:

        response.frame_Type = FRAME_RESPONSE_CONFIGURE_ACK;

        union{
            uint8_t byte_array[2];
            uint16_t int_value;
        }aux_bytes2uint16;

        aux_bytes2uint16.byte_array[0] = request.frame_Body[0];
        aux_bytes2uint16.byte_array[1] = request.frame_Body[1];
        flowMeter_setMeasurementTimeInterval(aux_bytes2uint16.int_value);
        response.frame_Body = aux_bytes2uint16.byte_array;
        response.frame_Length = 6;
    }

    // CRC is calculated inside Communications_send()
    if(response.frame_Type != 0){
        Communications_send(response);
    }
    return;
}


// Sets BUSY pin
void Communications_setBusy(){
    GPIO_setOutputHighOnPin(
        GPIO_PORT_P5,
        GPIO_PIN5
        );
}

// Clears BUSY pin
void Communications_clearBusy(){
    GPIO_setOutputLowOnPin(
        GPIO_PORT_P5,
        GPIO_PIN5
        );
}

// main function of Communications module, must be called on every iteration
void Communications_update(){

    if (SPI_slave.communication_Status == COMMUNICATION_STATUS_INACTIVE) {
        return;
    }

    if (SPI_slave.byte_Rx_received){
        SPI_slave.byte_Rx_received = false;

        if(SPI_slave.communication_Status == COMMUNICATION_STATUS_LISTENING && received_byte == '*'){
            __no_operation();

            SPI_slave.communication_Status = COMMUNICATION_STATUS_PROCESSING_REQUEST;
            SPI_slave.byte_Read_buffer[SPI_slave.byte_Read_Counter] = received_byte;
            SPI_slave.byte_Read_Counter++;
            return;
        }

        if(SPI_slave.communication_Status == COMMUNICATION_STATUS_PROCESSING_REQUEST){
            SPI_slave.byte_Read_buffer[SPI_slave.byte_Read_Counter] = received_byte;

            if(SPI_slave.byte_Read_Counter == 2 && SPI_slave.received_Frame_Length == 0){
                SPI_slave.received_Frame_Length = received_byte;
            }
            SPI_slave.byte_Read_Counter++;

            if(SPI_slave.byte_Read_Counter == (SPI_slave.received_Frame_Length) && SPI_slave.received_Frame_Length>0){
                EUSCI_A_SPI_disableInterrupt(EUSCI_A2_BASE,EUSCI_A_SPI_RECEIVE_INTERRUPT);
                SPI_slave.byte_Read_Counter = 0;
                SPI_slave.communication_Status = COMMUNICATION_STATUS_SENDING_RESPONSE;

                // Verify frame CRC (check if remainder != 0)
                if(Communications_CRC8(SPI_slave.byte_Read_buffer, SPI_slave.received_Frame_Length) != 0){
                    return;
                }

                SPI_slave.received_Frame_Length = 0;

                SPI_Communications_Frame request;
                request.frame_Type = SPI_slave.byte_Read_buffer[1];
                request.frame_Length = SPI_slave.byte_Read_buffer[2];
                request.frame_CRC = SPI_slave.byte_Read_buffer[SPI_slave.byte_Read_Counter - 1];

                // Check if received frame has a body (if length > 4 means there are more bytes than just head, type, length & CRC)
                if(request.frame_Length > 4){

                    // read buffer must be freed for incoming frames, allocate memory to store the current frame's body
                    uint8_t *body = malloc(request.frame_Length - 4);
                    for(uint8_t i = 0; i < (request.frame_Length - 4); i++){
                        body[i] = SPI_slave.byte_Read_buffer[3+i];
                    }
                    request.frame_Body = body;
                    Communications_ProcessRequest(request);
                    free(body);

                }else{
                    Communications_ProcessRequest(request);
                }
            }
        }
    }

    if (SPI_slave.byte_Tx_ready){
        SPI_slave.byte_Tx_ready = false;

        if(SPI_slave.communication_Status == COMMUNICATION_STATUS_SENDING_RESPONSE){

            // Check if the end of the frame was reached (counter >= frame length):
            if (SPI_slave.byte_Transmit_Counter >= SPI_slave.transmit_Frame_length){

                // Transmit zeroes to indicate end of communication:
                EUSCI_A_SPI_transmitData(EUSCI_A2_BASE,0);
                EUSCI_A_SPI_disableInterrupt(EUSCI_A2_BASE,EUSCI_A_SPI_TRANSMIT_INTERRUPT);
                SPI_slave.communication_Status = COMMUNICATION_STATUS_LISTENING;
                SPI_slave.byte_Transmit_Counter = 0;
                SPI_slave.transmit_Frame_length = 0;
                EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
                EUSCI_A_SPI_enableInterrupt(
                                           EUSCI_A2_BASE,
                                           EUSCI_A_SPI_RECEIVE_INTERRUPT
                                           );
                return;
            }

            // If not, transmit next byte & increase counter
            EUSCI_A_SPI_transmitData(EUSCI_A2_BASE, SPI_slave.byte_Transmit_buffer[SPI_slave.byte_Transmit_Counter] );
            SPI_slave.byte_Transmit_Counter++;

        }else{
            // If the system isn't ready to send a response or is unavailable, transmit zeroes:
            EUSCI_A_SPI_transmitData(EUSCI_A2_BASE,0);
        }
    return;
    }
}


/*  CRC8 implementation via lookup table. The returned value is set as the frame CRC byte
 *  this function is also used to validate if a frame is valid, by checking if the returned
 *  value equals zero. This is an implementation of the Dallas Semiconductor standard of CRC8
 *  and thus the used polynomial is 0x31, which is implicit on the results of the lookup table
 *  Based on the 2012 algorithm by Khusainov Timur. Link: https://github.com/timypik/Common-Library/blob/master/crc8.c
 */
uint8_t Communications_CRC8(uint8_t *data, uint8_t data_length){
    uint8_t crc_result = 0;

    // Data length can vary depending on whether CRC byte shall be included or not
    while(data_length--){
        crc_result = CRC8_TABLE[crc_result ^ *(data++)];
    }

    return crc_result;
}


