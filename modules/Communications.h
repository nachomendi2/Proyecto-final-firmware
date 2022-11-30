/*
 * Communications.h
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */

#ifndef MODULES_COMMUNICATIONS_H_
#define MODULES_COMMUNICATIONS_H_

#include <stdint.h>
#include <stdbool.h>

// Set buffer size for received bytes & bytes to transmit
#define SPI_RX_MAX_BUFFER_SIZE 5
#define SPI_TX_MAX_BUFFER_SIZE 30

/* Communication status state machine:
 * Communications are handled as a state machine with 4 possible states:
 *  COMMUNICATION_STATUS_INACTIVE: default state, changes to next state when SS=LOW
 *  COMMUNICATION_STATUS_LISTENING: next state after inactive, the module is waiting for a valid request
 *  COMMUNICATION_STATUS_PROCESSING_REQUEST: next state after listening, it occurs when a * is received while listening
 *  COMMUNICATION_STATUS_SENDING_RESPONSE: final state, after all request bytes have been received. Transmit response bytes y jump to listening
 */
typedef enum
{
    COMMUNICATION_STATUS_INACTIVE,
    COMMUNICATION_STATUS_LISTENING,
    COMMUNICATION_STATUS_PROCESSING_REQUEST,
    COMMUNICATION_STATUS_SENDING_RESPONSE
}SPI_Communication_Status;

typedef struct
{
    SPI_Communication_Status communication_Status;
    uint8_t byte_Read_Counter;
    uint8_t byte_Transmit_Counter;
    uint8_t byte_Read_buffer[SPI_RX_MAX_BUFFER_SIZE];
    uint8_t byte_Transmit_buffer[SPI_TX_MAX_BUFFER_SIZE];
    uint8_t received_Frame_Length;
    uint8_t transmit_Frame_length;
    bool byte_Rx_received;
    bool byte_Tx_ready;
}SPI_Communications_Handler;

/* A Communications frame is composed of the following fields:
 * - Header (1 byte)
 * - Frame type (1 byte)
 * - Frame Length (1 byte)
 * - Body (N bytes as indicated in frame length)
 * - CRC error correction (1 byte)
 */


typedef enum
{
    FRAME_REQUEST_TOTALIZER = 0x01,
    FRAME_REQUEST_STATUS = 0x03,
    FRAME_REQUEST_MEASUREMENT = 0x05,
    FRAME_REQUEST_AVERAGE_MASS_FLOW_RATE = 0x07,
    FRAME_REQUEST_OPEN_VALVE = 0xA0,
    FRAME_REQUEST_CLOSE_VALVE = 0xAF,
    FRAME_REQUEST_CONFIGURE = 0x10,
    FRAME_RESPONSE_TOTALIZER = 0x02,
    FRAME_RESPONSE_STATUS = 0x04,
    FRAME_RESPONSE_AVERAGE_MASS_FLOW_RATE = 0x08,
    FRAME_RESPONSE_VALVE_ACK = 0xA5,
}SPI_Communications_Frame_Type;

typedef struct
{
    SPI_Communications_Frame_Type frame_Type;
    uint8_t frame_Length;
    uint8_t frame_CRC;
    uint8_t *frame_Body;
}SPI_Communications_Frame;

void Communications_setup(void);

bool Communications_send(SPI_Communications_Frame frame);

bool Communications_isActive();


#endif /* MODULES_COMMUNICATIONS_H_ */
