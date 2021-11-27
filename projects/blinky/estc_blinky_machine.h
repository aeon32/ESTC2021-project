#ifndef ESTC_BLINKY_MACHINE_H
#define ESTC_BLINKY_MACHINE_H

#include <nrfx_systick.h>

typedef struct
{
	nrfx_systick_state_t current_pwm_value_start_timestamp;
	bool brightness_increasing;
	uint32_t sequence_step;
	uint32_t pwm_max_value;
	uint32_t blink_period;
	const uint32_t* sequence_table;
	size_t sequence_table_size;
	uint32_t pwm_value;
} ESTCBlinkyMachine;

/**
 * Initialize PWM for pin. Pin must be configured for output.
 * sequence_table must exist during lifetime of blinky_machine
**/
void
estc_blinky_machine_init(ESTCBlinkyMachine* blinky_machine, const uint32_t* sequence_table, size_t sequence_table_size,
		uint32_t blink_period, uint32_t pwm_max_value);

/**
 * Set new PWM value
 * 
**/
void estc_blinky_machine_next_state(ESTCBlinkyMachine* blinky_machine);

/**
 * Return pwm value for led
**/
uint32_t estc_blinky_machine_get_led_pwm(ESTCBlinkyMachine* blinky_machine, uint32_t led_number);

#endif