#include "estc_monotonic_time.h"
#include <nrf_atomic.h>
#include <app_timer.h>
#include <nrf_log.h>

static nrf_atomic_u32_t uptime = 0;

void estc_monotonic_time_update(uint32_t add_usec)
{
	nrf_atomic_u32_add(&uptime, add_usec);
}

ESTCTimeStamp estc_monotonic_time_get()
{
	return nrf_atomic_u32_fetch_add(&uptime, 0);
}

bool estc_monotonic_time_elapsed_test(ESTCTimeStamp start_time, uint32_t interval)
{
	return (nrf_atomic_u32_fetch_add(&uptime, 0) - start_time) >= interval;
}

void estc_monotonic_time_set(ESTCTimeStamp time)
{
	nrf_atomic_u32_store(&uptime, time);
}

uint32_t estc_monotonic_time_diff(ESTCTimeStamp start_time, ESTCTimeStamp end_time)
{
	return end_time - start_time;
}

static void rtc_monotonic_time_handler(void* p_context)
{
	estc_monotonic_time_update(1);
}

void estc_monotonic_time_start_update_timer()
{
	ret_code_t err = app_timer_init();
	APP_ERROR_CHECK(err);
	APP_TIMER_DEF(monotonic_time_timer_id);
	err = app_timer_create(&monotonic_time_timer_id, APP_TIMER_MODE_REPEATED, rtc_monotonic_time_handler);
	err = app_timer_start(monotonic_time_timer_id, APP_TIMER_CLOCK_FREQ, NULL);
}