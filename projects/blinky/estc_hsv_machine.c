#include "estc_hsv_machine.h"

#include <boards.h>
#include <nrf_log.h>
#include <assert.h>


static const int SLOW_BLINK_FREQUENCY = 1;
//period of blink 
static const int SLOW_BLINK_PERIOD = 1000000 / SLOW_BLINK_FREQUENCY;

static const int FAST_BLINK_FREQUENCY = 2;
//period of blink 
static const int FAST_BLINK_PERIOD = 1000000 / FAST_BLINK_FREQUENCY;

//component increasing period, from 0 to pwm_max
static const int COMPONENT_INCREASING_PERIOD = 5000000;

/**
 * 
 * 
**/ 
void estc_hsv_machine_init(ESTCHSVMachine* hsv_machine, uint32_t pwm_max_value)
{
    hsv_machine->mode_led_current_pwm_value_start_timestamp = estc_monotonic_time_get();
    hsv_machine->component_increasing_step_timestamp = hsv_machine->mode_led_current_pwm_value_start_timestamp;
    
    hsv_machine->pwm_max_value = pwm_max_value;
    hsv_machine->mode = ESTCHSV_NO_INPUT;

    hsv_machine->mode_led_blink_period = 0;
    hsv_machine->mode_led_brightness_increasing = true;
    memset(&hsv_machine->pwm_values, 0, sizeof(hsv_machine->pwm_values));
}


/**
 * Switch working mode
**/ 
void estc_hsv_machine_switch_mode(ESTCHSVMachine * hsv_machine)
{
    switch(hsv_machine->mode)
    {
        case ESTCHSV_NO_INPUT:
           hsv_machine->mode++;
           hsv_machine->mode_led_blink_period = SLOW_BLINK_PERIOD;
        break;
        case ESTCHSV_HUE:
           hsv_machine->mode++;
           hsv_machine->mode_led_blink_period = FAST_BLINK_PERIOD;
        break;
        case ESTCHSV_SATURATION:
           hsv_machine->mode++;
        break;
        case ESTCHSV_BRIGHTNESS:
           hsv_machine->mode = ESTCHSV_NO_INPUT;

    }

}


static void estc_hsv_machine_calculate_mode_led_pwm(ESTCHSVMachine* hsv_machine, ESTCTimeStamp current_time)
{
    if(hsv_machine->mode == ESTCHSV_NO_INPUT)
    {
        hsv_machine->pwm_values[0] = 0;
        hsv_machine->mode_led_current_pwm_value_start_timestamp = current_time;
    
    } else if(hsv_machine->mode == ESTCHSV_BRIGHTNESS) 
    {
    
        hsv_machine->pwm_values[0] = hsv_machine->pwm_max_value;
        hsv_machine->mode_led_current_pwm_value_start_timestamp = current_time;
    
    } else 
    {
        uint32_t time_elapsed = estc_monotonic_time_diff(hsv_machine->mode_led_current_pwm_value_start_timestamp, current_time);
        if (time_elapsed >= hsv_machine->mode_led_blink_period / hsv_machine->pwm_max_value /2)
        {
            hsv_machine->mode_led_current_pwm_value_start_timestamp = current_time;
            if (hsv_machine->mode_led_brightness_increasing)
            {
                if (hsv_machine->pwm_values[0] == hsv_machine->pwm_max_value)
                {
                   //now led is fading..
                   hsv_machine->mode_led_brightness_increasing = false;
                 } else {
                   hsv_machine->pwm_values[0]++;
               }                

            }
            if (!hsv_machine->mode_led_brightness_increasing)
            {
                if (hsv_machine->pwm_values[0] == 0)
                {

                    hsv_machine->mode_led_brightness_increasing= true;

                } else {
                   
                   hsv_machine->pwm_values[0]--;

                }            
            }
        }
    }     

}


/**
 * Set new state
 * 
**/ 
void estc_hsv_machine_next_state(ESTCHSVMachine* hsv_machine)
{
    ESTCTimeStamp current_time = estc_monotonic_time_get();
    estc_hsv_machine_calculate_mode_led_pwm(hsv_machine, current_time);
 
}

void estc_hsv_machine_increase_component(ESTCHSVMachine * hsv_machine)
{

    if (hsv_machine->mode > 0)
    {
        ESTCTimeStamp current_time = estc_monotonic_time_get();
        uint32_t time_elapsed = estc_monotonic_time_diff(hsv_machine->component_increasing_step_timestamp, current_time);
        if (time_elapsed >= COMPONENT_INCREASING_PERIOD / hsv_machine->pwm_max_value )
        {
            hsv_machine->component_increasing_step_timestamp = current_time;
            hsv_machine->pwm_values[hsv_machine->mode]++;
            if (hsv_machine->pwm_values[hsv_machine->mode] > hsv_machine->pwm_max_value)
            {
                hsv_machine->pwm_values[hsv_machine->mode] = 0;

            }

        }

    }


}


uint32_t estc_hsv_machine_get_led_pwm(ESTCHSVMachine* hsv_machine, uint32_t led_number)
{

    assert(led_number < HSV_MACHINE_LEDS);
    return hsv_machine->pwm_values[led_number];

}