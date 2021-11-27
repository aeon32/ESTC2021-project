#include "estc_pwm.h"

#include <boards.h>

/**
 * Initialize PWM for pin. Pin must be configured for output.
 * 
**/
void estc_pwm_init(ESTCPWM* pwm, uint8_t pin, bool inverse_pin, uint32_t pwm_period, uint32_t pwm_max_value)
{
	pwm->pin = pin;
	nrfx_systick_get(&pwm->pwm_period_start_timestamp);
	pwm->pwm_value = 0;
	pwm->pwm_new_value = 0;
	pwm->pin_switched_on = true;
	pwm->inverse_pin = inverse_pin;
	pwm->pwm_period = pwm_period;
	pwm->pwm_max_value = pwm_max_value;
}

/**
 * Set new PWM value
 * 
**/
void estc_pwm_set_value(ESTCPWM* pwm, uint32_t pwm_value)
{
	pwm->pwm_new_value = pwm_value;
}

/**
 *  Toggle pin, performing pwm operation. Should be called in loop with minimum delays between calls.
**/
void estc_pwm_handle(ESTCPWM* pwm)
{
	if (pwm->pin_switched_on)
	{
		uint32_t pin_on_timeout = pwm->pwm_value * pwm->pwm_period / pwm->pwm_max_value;
		if (nrfx_systick_test(&pwm->pwm_period_start_timestamp, pin_on_timeout))
		{
			if (pwm->inverse_pin)
				nrf_gpio_pin_set(pwm->pin);
			else
				nrf_gpio_pin_clear(pwm->pin);
			pwm->pin_switched_on = false;
		}
	}
	else
	{
		uint32_t pin_off_timeout = pwm->pwm_period;
		if (nrfx_systick_test(&pwm->pwm_period_start_timestamp, pin_off_timeout))
		{
			//next pwm period started..
			nrfx_systick_get(&pwm->pwm_period_start_timestamp);
			pwm->pwm_value = pwm->pwm_new_value;

			if (pwm->pwm_value > 0)
			{
				if (pwm->inverse_pin)
					nrf_gpio_pin_clear(pwm->pin);
				else
					nrf_gpio_pin_set(pwm->pin);
			}
			pwm->pin_switched_on = true;

		}
	}
}
