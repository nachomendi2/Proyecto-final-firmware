/*
 * Communications.h
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */

#ifndef MODULES_COMMUNICATIONS_H_
#define MODULES_COMMUNICATIONS_H_

#include <stdint.h>

typedef enum
{
    COMMUNICATION_STATUS_INACTIVE,
    COMMUNICATION_STATUS_ACTIVE
}CommunicationStatus;

void Communications_setup(void);

void Communications_send(uint8_t transmit_data);

bool Communications_isActive();

uint8_t Communications_read();

#endif /* MODULES_COMMUNICATIONS_H_ */
