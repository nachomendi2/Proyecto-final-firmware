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
    bool restartAfterCompletion;
    UT_TMR_DELAY_STATE state;
}ut_tmrDelay_t;

bool UT_timer_delay(ut_tmrDelay_t *timer_delay);

#endif /* UTILS_H_ */
