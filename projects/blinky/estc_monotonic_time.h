#ifndef ESTC_MONOTONIC_TIME
#define ESTC_MONOTONIC_TIME

#include <nrfx_systick.h>

/**
 *  Timestamp. Time in usec
**/
typedef uint32_t ESTCTimeStamp;

/**
 *  Update internal timestamp.
 *  Add usec to internal 32-bit counter. Should be called from time interrupt.
 *  Overflows every 4294 secs
**/
void estc_monotonic_time_update(uint32_t add_usec);

/**
 *  Returns value of internal 32-bit counter.
 *  Approximetely would be equal to current uptime in msec, 
 *     if we have been calling estc_monotonic_time periodically.
**/
ESTCTimeStamp estc_monotonic_time_get();

/**
 *  Test if period usec is over since start_time timestamp
**/
bool estc_monotonic_time_elapsed_test(ESTCTimeStamp start_time, uint32_t interval );

/**
 *  Set uptime value
**/
void estc_monotonic_time_set(ESTCTimeStamp time);


/**
 * Initialize and start update timer, based on RTC
 * RTC must be enabled sdk_config.h
**/

void estc_monotonic_time_start_update_timer();

#endif