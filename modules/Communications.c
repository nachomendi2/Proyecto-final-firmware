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


CommunicationStatus com_status;
uint8_t response = 0;

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
    GPIO_setAsInputPinWithPullDownResistor(
        GPIO_PORT_P2,
        GPIO_PIN2
        );

    GPIO_enableInterrupt(
        GPIO_PORT_P2,
        GPIO_PIN2
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

    // 6. Enable Interrupt & clear flags (just in case)
    EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);

    EUSCI_A_SPI_enableInterrupt(
                           EUSCI_A2_BASE,
                           EUSCI_A_SPI_RECEIVE_INTERRUPT
                           );

}

void Communications_send(uint8_t transmit_data){
    EUSCI_A_SPI_transmitData(EUSCI_A2_BASE, transmit_data);
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
   switch(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN2)){
       case GPIO_INPUT_PIN_LOW:
           com_status = COMMUNICATION_STATUS_ACTIVE;
           break;
       case GPIO_INPUT_PIN_HIGH:

           com_status = COMMUNICATION_STATUS_INACTIVE;
           EUSCI_A_SPI_disableInterrupt(
                   EUSCI_A2_BASE,
                   EUSCI_A_SPI_RECEIVE_INTERRUPT
                   );
           break;
   }
   GPIO_clearInterrupt(GPIO_PORT_P2,GPIO_PIN2);
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
    EUSCI_A_SPI_clearInterrupt(EUSCI_A2_BASE,EUSCI_A_SPI_TRANSMIT_INTERRUPT + EUSCI_A_SPI_RECEIVE_INTERRUPT);
    response = Communications_read();
    Communications_send(120);
}

bool Communications_isActive(){
    if (com_status == COMMUNICATION_STATUS_ACTIVE) {
        return true;
    }else{
        return false;
    }
}

uint8_t Communications_read(){
    return EUSCI_A_SPI_receiveData(EUSCI_A2_BASE);
}

