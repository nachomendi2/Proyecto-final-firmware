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
#define SPI_TX_MAX_BUFFER_SIZE 40

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
}SPI_Communication_Status_t;

typedef struct
{
    SPI_Communication_Status_t communication_Status;
    uint8_t byte_Read_Counter;
    uint8_t byte_Transmit_Counter;
    uint8_t byte_Read_buffer[SPI_RX_MAX_BUFFER_SIZE];
    uint8_t byte_Transmit_buffer[SPI_TX_MAX_BUFFER_SIZE];
    uint8_t received_Frame_Length;
    uint8_t transmit_Frame_length;
    bool byte_Rx_received;
    bool byte_Tx_ready;
}SPI_Communications_Module;

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

void Communications_ProcessRequest(SPI_Communications_Frame request);

void Communications_update();

void Communications_setBusy();

void Communications_clearBusy();

uint8_t Communications_CRC8(uint8_t *data, uint8_t data_length);

/*
 * Lookup table for fast CRC calculation, polynomial used for the calculation
 * of these values was 0x31 according to Dallas Semiconductor standard.
 */
static const uint8_t CRC8_TABLE[] =
{
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
    157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
     35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
    190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
     70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
    219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
    101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
    248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
    140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
     17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
    175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
     50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
    202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
     87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
    233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
    116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};



#endif /* MODULES_COMMUNICATIONS_H_ */
