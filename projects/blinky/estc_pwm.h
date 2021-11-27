#ifndef ESTC_PWM_H
#define ESTC_PWM_H

#include <nrfx_systick.h>

typedef struct
{
	uint8_t pin;
	nrfx_systick_state_t pwm_period_start_timestamp;
	uint32_t pwm_period;
	uint32_t pwm_max_value;
	uint32_t pwm_value;
	uint32_t pwm_new_value;
	bool brightness_increasing;
	bool inverse_pin;
	bool pin_switched_on;

} ESTCPWM;

/**
 * Initialize PWM for pin. Pin must be configured for output.
 * 
**/
void estc_pwm_init(ESTCPWM* pwm, uint8_t pin, bool inverse_pin, uint32_t pwm_period, uint32_t pwm_max_value);

/**
 * Set new PWM value
 * 
**/
void estc_pwm_set_value(ESTCPWM* pwm, uint32_t pwm_value);

/**
 *  Toggle pin, performing pwm operation. Should be called in loop with minimum delays between calls.
**/
void estc_pwm_handle(ESTCPWM* pwm);

#endif