#ifndef ESTC_HSV_MACHINE_H
#define ESTC_HSV_MACHINE_H

#include "estc_monotonic_time.h"

#define HSV_MACHINE_LEDS 4
#define HSV_COMPONENTS 3

typedef enum _ESTCHSVMachineMode
{
	ESTCHSV_NO_INPUT = 0,
	ESTCHSV_HUE = 1,
	ESTCHSV_SATURATION = 2,
	ESTCHSV_BRIGHTNESS = 3
} ESTCHSVMachineMode;

typedef struct
{
	ESTCTimeStamp mode_led_current_pwm_value_start_timestamp;
	ESTCTimeStamp component_increasing_step_timestamp;
	ESTCHSVMachineMode mode;

	bool mode_led_brightness_increasing;
	uint32_t mode_led_blink_period;

	uint32_t pwm_max_value;
	uint32_t pwm_values[HSV_MACHINE_LEDS];
	int hsv_components[HSV_COMPONENTS];
} ESTCHSVMachine;

/**
 * Initialize PWM for pin. Pin must be configured for output.
 * sequence_table must exist during lifetime of blinky_machine
**/
void estc_hsv_machine_init(ESTCHSVMachine* hsv_machine, uint32_t pwm_max_value);

/**
 * Switch working mode
**/
void estc_hsv_machine_switch_mode(ESTCHSVMachine* hsv_machine);

/**
 * Increase one of components, according to current mode
**/
void estc_hsv_machine_increase_component(ESTCHSVMachine* hsv_machine);

/**
 * Set new PWM value
 * 
**/
void estc_hsv_machine_next_state(ESTCHSVMachine* blinky_machine);

/**
 * Return pwm value for led
**/
uint32_t estc_hsv_machine_get_led_pwm(ESTCHSVMachine* blinky_machine, uint32_t led_number);

#endif