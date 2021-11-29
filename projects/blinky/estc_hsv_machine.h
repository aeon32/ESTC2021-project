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

typedef void (*estc_hsv_machine_toggle_mode_handler) (ESTCHSVMachineMode new_mode, void * user_data);


typedef union 
{
    int32_t hsv_components[HSV_COMPONENTS];
    struct {
        int32_t h;
        int32_t s;
        int32_t v;
    } hsv;
} HSVColor;


typedef struct
{
    ESTCTimeStamp mode_led_current_pwm_value_start_timestamp;
    ESTCTimeStamp component_increasing_step_timestamp;
    ESTCHSVMachineMode mode;

    bool mode_led_brightness_increasing;
    uint32_t mode_led_blink_period;

    uint32_t pwm_max_value;
    uint32_t pwm_values[HSV_MACHINE_LEDS];
    HSVColor led_color;
    bool hsv_components_increasing[HSV_COMPONENTS];
    estc_hsv_machine_toggle_mode_handler toggle_mode_handler;
    void * user_data;
} ESTCHSVMachine;



/**
 * Initialize hsv machine
 * @param hsv_components - initial values of hsv_components
**/
void estc_hsv_machine_init(ESTCHSVMachine* hsv_machine, const HSVColor * led_color, 
    uint32_t pwm_max_value, estc_hsv_machine_toggle_mode_handler toggle_mode_handler, void * user_data);

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
void estc_hsv_machine_next_state(ESTCHSVMachine* hsv_machine);

/**
 * Return pwm value for led
**/
uint32_t estc_hsv_machine_get_led_pwm(ESTCHSVMachine* hsv_machine, uint32_t led_number);


/**
 * Return hsv values array
**/
HSVColor estc_hsv_machine_get_components(ESTCHSVMachine* blinky_machine);

#endif