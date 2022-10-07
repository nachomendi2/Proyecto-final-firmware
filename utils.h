/*
 * utils.h
 *
 *  Created on: 26 sep. 2022
 *      Author: ignaciomendizabal
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    UT_TMR_DELAY_INIT,
    UT_TMR_DELAY_WAIT
}UT_TMR_DELAY_STATE;

typedef struct
{
    uint64_t startValue;
    UT_TMR_DELAY_STATE state;
}ut_tmrDelay_t;

bool UT_delayms(ut_tmrDelay_t* p_timer, uint32_t p_ms);

#endif /* UTILS_H_ */
