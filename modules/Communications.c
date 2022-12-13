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

SPI_Communications_Module SPI_slave = {0};

void Communications_setup(void){

    WDT_A_hold(WDT_A_BASE);

    /* 1. Configure pins for eUSCI_A2:
     * Change Peripheral Module Function of pins 0,1,2,3,4,5 of PORT J to use eUSCI_A2 instead
     *  - Set PIN 0 as Input (CLK)
     *  - Set PIN 1 as Input (STE) (Unused)
     *  - Set PIN 2 as Input (MOSI)
     *  - Set PIN 3 as Output (MISO)
     */
    GPIO_setAsOutputPin(GPIO_PORT_PJ, GPIO_PIN3);
    GPIO_setAsInputPin(GPIO_PORT_PJ, GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2);

    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_PJ,
        GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2,
        GPIO_PRIMARY_MODULE_FUNCTION
        );

    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_PJ,
        GPIO_PIN3,
        GPIO_PRIMARY_MODULE_FUNCTION
        );

    //2. Configure GPIO for slave select:
    GPIO_setAsInputPin(
        GPIO_PORT_P2,
        GPIO_PIN2
        );

    GPIO_enableInterrupt(
        GPIO_PORT_P2,
        GPIO_PIN2
        );

    GPIO_selectInterruptEdge(
        GPIO_PORT_P2,
        GPIO_PIN2,
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
    params.clockPhase = EUSCI_A_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
    params.clockPolarity = EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    params.spiMode = EUSCI_A_SPI_3PIN;

    EUSCI_A_SPI_initSlave(EUSCI_A2_BASE, &params);

    // 5. Start eUSCI_A2 module:
    EUSCI_A_SPI_enable(EUSCI_A2_BASE);

    // Aditional: 6. configure busy pin (PORT 2.3)
    GPIO_setAsOutputPin(
        GPIO_PORT_P2,
        GPIO_PIN3
        );
}

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

    SPI_slave.byte_Transmit_buffer[frame.frame_Length - 1] = frame.frame_CRC;

    // Transmit first byte
    //EUSCI_A_SPI_transmitData(EUSCI_A2_BASE, SPI_slave.byte_Transmit_buffer[SPI_slave.byte_Transmit_Counter] );
    //SPI_slave.byte_Transmit_Counter++;

    // Enable transmit interrupts
    EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);

    EUSCI_A_SPI_enableInterrupt(
                               EUSCI_A2_BASE,
                               EUSCI_A_SPI_TRANSMIT_INTERRUPT
                               );

    return true;
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = PORT2_VECTOR
__interrupt void Port2_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT2_VECTOR))) Port2_ISR (void)
#else
#error Compiler not supported!
#endif
{
   GPIO_clearInterrupt(GPIO_PORT_P2,GPIO_PIN2);
   switch(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN2)){
       case GPIO_INPUT_PIN_LOW:
           SPI_slave.communication_Status = COMMUNICATION_STATUS_LISTENING;
           EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
           EUSCI_A_SPI_enableInterrupt(
                                      EUSCI_A2_BASE,
                                      EUSCI_A_SPI_RECEIVE_INTERRUPT
                                      );
           GPIO_selectInterruptEdge(
                   GPIO_PORT_P2,
                   GPIO_PIN2,
                   GPIO_LOW_TO_HIGH_TRANSITION
                   );
           break;
       case GPIO_INPUT_PIN_HIGH:

           SPI_slave.communication_Status = COMMUNICATION_STATUS_INACTIVE;
           EUSCI_A_SPI_disableInterrupt(
                   EUSCI_A2_BASE,
                   EUSCI_A_SPI_RECEIVE_INTERRUPT
                   );
           GPIO_selectInterruptEdge(
                   GPIO_PORT_P2,
                   GPIO_PIN2,
                   GPIO_HIGH_TO_LOW_TRANSITION
                   );
           break;
   }
}

uint8_t received_byte = 0;

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A2_VECTOR
__interrupt void USCI_A2_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(EUSCI_A2_VECTOR))) USCI_A2_ISR (void)
#else
#error Compiler not supported!
#endif
{
    if (EUSCI_A_SPI_getInterruptStatus(
            EUSCI_A2_BASE,
            EUSCI_A_SPI_TRANSMIT_INTERRUPT) == EUSCI_A_SPI_TRANSMIT_INTERRUPT) {
        EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE,EUSCI_A_SPI_TRANSMIT_INTERRUPT);
        SPI_slave.byte_Tx_ready = true;
        return;
    }

    if (EUSCI_A_SPI_getInterruptStatus(
                EUSCI_A2_BASE,
                EUSCI_A_SPI_RECEIVE_INTERRUPT) == EUSCI_A_SPI_RECEIVE_INTERRUPT) {
        EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE,EUSCI_A_SPI_RECEIVE_INTERRUPT);
        SPI_slave.byte_Rx_received = true;
        return;
    }

}

bool Communications_isActive(){
    if (SPI_slave.communication_Status != COMMUNICATION_STATUS_INACTIVE) {
        return true;
    }else{
        return false;
    }
}

SPI_Communications_Frame Communications_ProcessFrame(SPI_Communications_Frame request){

    // Communications_CheckCRC();
    SPI_Communications_Frame response;

    switch(request.frame_Type){
    case FRAME_REQUEST_AVERAGE_MASS_FLOW_RATE:
        response.frame_Type = FRAME_RESPONSE_AVERAGE_MASS_FLOW_RATE;
        response.frame_CRC = 40;
        response.frame_Length = 8;

        static union {
                    float volumeFlowRate;
                    uint8_t b[4];
                }body_afr;

        body_afr.volumeFlowRate = flowMeter_getTotalizer();
        response.frame_Body = body_afr.b;
    case FRAME_REQUEST_TOTALIZER:
        response.frame_Type = FRAME_RESPONSE_TOTALIZER;
        response.frame_CRC = 40;
        response.frame_Length = 8;

        static union {
            float volumeFlowRate;
            uint8_t b[4];
        }body_totalizer;

        body_totalizer.volumeFlowRate = flowMeter_getAverageMassFlowRate();
        response.frame_Body = body_totalizer.b;
        break;
    case FRAME_REQUEST_OPEN_VALVE:
        Communications_setBusy();
        response.frame_Type = FRAME_RESPONSE_VALVE_ACK;
        response.frame_CRC = 40;
        response.frame_Length = 5;
        response.frame_Body = valveControl_getValveState();
        valveControl_open();
        break;
    case FRAME_REQUEST_CLOSE_VALVE:
        Communications_setBusy();
        response.frame_Type = FRAME_RESPONSE_VALVE_ACK;
        response.frame_CRC = 40;
        response.frame_Length = 5;
        response.frame_Body = valveControl_getValveState();
        valveControl_close();
        break;
    }

    return response;
}

void Communications_setBusy(){
    GPIO_setOutputHighOnPin(
        GPIO_PORT_P2,
        GPIO_PIN3
        );
}

void Communications_clearBusy(){
    GPIO_setOutputLowOnPin(
        GPIO_PORT_P2,
        GPIO_PIN3
        );
}

void Communications_update(){

    if (SPI_slave.communication_Status == COMMUNICATION_STATUS_INACTIVE) {
            return;
        }

    if (SPI_slave.byte_Rx_received){
        SPI_slave.byte_Rx_received = false;
        uint8_t received_byte = EUSCI_A_SPI_receiveData(EUSCI_A2_BASE);

        if(SPI_slave.communication_Status == COMMUNICATION_STATUS_LISTENING && received_byte == '*'){

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

            if(SPI_slave.byte_Read_Counter == (SPI_slave.received_Frame_Length - 1) && SPI_slave.received_Frame_Length>0){
                EUSCI_A_SPI_disableInterrupt(EUSCI_A2_BASE,EUSCI_A_SPI_RECEIVE_INTERRUPT);
                SPI_slave.byte_Read_Counter = 0;
                SPI_slave.communication_Status = COMMUNICATION_STATUS_SENDING_RESPONSE;
                SPI_Communications_Frame request;
                request.frame_Type = SPI_slave.byte_Read_buffer[1];
                request.frame_Length = SPI_slave.byte_Read_buffer[2];
                SPI_slave.received_Frame_Length = 0;
                request.frame_CRC = SPI_slave.byte_Read_buffer[SPI_slave.byte_Read_Counter - 1];
                SPI_Communications_Frame response = Communications_ProcessFrame(request);
                Communications_send(response);
            }
        }
    }

    if (SPI_slave.byte_Tx_ready){
        SPI_slave.byte_Tx_ready = false;

        if(SPI_slave.communication_Status == COMMUNICATION_STATUS_SENDING_RESPONSE){
            if(SPI_slave.byte_Transmit_Counter < SPI_slave.transmit_Frame_length){
                EUSCI_A_SPI_transmitData(EUSCI_A2_BASE, SPI_slave.byte_Transmit_buffer[SPI_slave.byte_Transmit_Counter] );
                SPI_slave.byte_Transmit_Counter++;
            }

            if (SPI_slave.byte_Transmit_Counter == SPI_slave.transmit_Frame_length){
                EUSCI_A_SPI_disableInterrupt(EUSCI_A2_BASE,EUSCI_A_SPI_TRANSMIT_INTERRUPT);
                SPI_slave.communication_Status = COMMUNICATION_STATUS_LISTENING;
                SPI_slave.byte_Transmit_Counter = 0;
                SPI_slave.transmit_Frame_length = 0;
                EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
                EUSCI_A_SPI_enableInterrupt(
                                           EUSCI_A2_BASE,
                                           EUSCI_A_SPI_RECEIVE_INTERRUPT
                                           );
            }
        }else{
            EUSCI_A_SPI_transmitData(EUSCI_A2_BASE,0);
        }
    return;
    }
}

