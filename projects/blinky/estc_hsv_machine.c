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

//max values in hsv model
static int MAX_COMPONENT_VALUES[3] = { 255, 255, 255 };

static void estc_hsv_machine_calculate_rgb_values(ESTCHSVMachine* hsv_machine);

void estc_hsv_machine_init(ESTCHSVMachine* hsv_machine, const HSVColor * led_color, 
    uint32_t pwm_max_value, estc_hsv_machine_toggle_mode_handler toggle_mode_handler, void * user_data)
{
    hsv_machine->mode_led_current_pwm_value_start_timestamp = estc_monotonic_time_get();
    hsv_machine->component_increasing_step_timestamp = hsv_machine->mode_led_current_pwm_value_start_timestamp;
    hsv_machine->pwm_max_value = pwm_max_value;
    hsv_machine->mode = ESTCHSV_NO_INPUT;
    hsv_machine->mode_led_blink_period = 0;
    hsv_machine->mode_led_brightness_increasing = true;
    hsv_machine->led_color = *led_color;
    for (int i = 0; i < HSV_COMPONENTS; i++)
    {
        hsv_machine->hsv_components_increasing[i] = !(led_color->hsv_components[i] >= MAX_COMPONENT_VALUES[i]);
        hsv_machine->pwm_values[i] = 0;
    }
    estc_hsv_machine_calculate_rgb_values(hsv_machine);
    hsv_machine->toggle_mode_handler = toggle_mode_handler;
    hsv_machine->user_data = user_data;
}

/**
 * Switch working mode
**/
void estc_hsv_machine_switch_mode(ESTCHSVMachine* hsv_machine)
{
    switch (hsv_machine->mode)
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
    if (hsv_machine->toggle_mode_handler)
    {
        hsv_machine->toggle_mode_handler(hsv_machine->mode, hsv_machine->user_data);
    }
}

static void estc_hsv_machine_calculate_mode_led_pwm(ESTCHSVMachine* hsv_machine, ESTCTimeStamp current_time)
{
    if (hsv_machine->mode == ESTCHSV_NO_INPUT)
    {
        hsv_machine->pwm_values[0] = 0;
        hsv_machine->mode_led_current_pwm_value_start_timestamp = current_time;
    }
    else if (hsv_machine->mode == ESTCHSV_BRIGHTNESS)
    {
        hsv_machine->pwm_values[0] = hsv_machine->pwm_max_value;
        hsv_machine->mode_led_current_pwm_value_start_timestamp = current_time;
    }
    else
    {
        uint32_t time_elapsed = estc_monotonic_time_diff(hsv_machine->mode_led_current_pwm_value_start_timestamp,
                current_time);
        if (time_elapsed >= hsv_machine->mode_led_blink_period / hsv_machine->pwm_max_value / 2)
        {
            hsv_machine->mode_led_current_pwm_value_start_timestamp = current_time;
            if (hsv_machine->mode_led_brightness_increasing)
            {
                if (hsv_machine->pwm_values[0] == hsv_machine->pwm_max_value)
                {
                    //now led is fading..
                    hsv_machine->mode_led_brightness_increasing = false;
                }
                else
                {
                    hsv_machine->pwm_values[0]++;
                }
            }
            if (!hsv_machine->mode_led_brightness_increasing)
            {
                if (hsv_machine->pwm_values[0] == 0)
                {
                    hsv_machine->mode_led_brightness_increasing = true;
                }
                else
                {
                    hsv_machine->pwm_values[0]--;
                }
            }
        }
    }
}

//Some stackoverflow magic )
static void hsv_to_rgb(int32_t * in_hsv_values, uint32_t* out_rgb_values)
{
    uint32_t region, remainder, p, q, t;
    int h = in_hsv_values[0];
    int s = in_hsv_values[1];
    int v = in_hsv_values[2];

    if (s == 0)
    {
        out_rgb_values[0] = v;
        out_rgb_values[1] = v;
        out_rgb_values[2] = v;
        return;
    }
    region = h / 43;
    remainder = (h - (region * 43)) * 6;
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s *  (255 - remainder)) >> 8))) >> 8;
    switch (region)
    {
    case 0:
        out_rgb_values[0] = v;
        out_rgb_values[1] = t;
        out_rgb_values[2] = p;
        break;
    case 1:
        out_rgb_values[0] = q;
        out_rgb_values[1] = v;
        out_rgb_values[2] = p;
        break;
    case 2:
        out_rgb_values[0] = p;
        out_rgb_values[1] = v;
        out_rgb_values[2] = t;
        break;
    case 3:
        out_rgb_values[0] = p;
        out_rgb_values[1] = q;
        out_rgb_values[2] = v;
        break;
    case 4:
        out_rgb_values[0] = t;
        out_rgb_values[1] = p;
        out_rgb_values[2] = v;
        break;
    default:
        out_rgb_values[0] = v;
        out_rgb_values[1] = p;
        out_rgb_values[2] = q;
        break;
    }
}

void rgb_to_hsv(const RGBColor * in_rgb, HSVColor * out_hsv)
{
    int32_t rgb_min = in_rgb->rgb_components[0];
    int32_t rgb_max = in_rgb->rgb_components[0];
    memset(out_hsv, 0, sizeof(HSVColor));
    
    for (int i = 1; i < RGB_COMPONENTS; i++)
    {
        if (in_rgb->rgb_components[i] < rgb_min)
        {
            rgb_min = in_rgb->rgb_components[i];
        }
        if (in_rgb->rgb_components[i] > rgb_max)
        {
            rgb_max = in_rgb->rgb_components[i];
        }        
    }

    out_hsv->hsv.v = rgb_max;
    if (out_hsv->hsv.v == 0)
    {
        out_hsv->hsv.h = 0;
        out_hsv->hsv.s = 0;
        return;
    }

    out_hsv->hsv.s = 255 * (rgb_max - rgb_min) / out_hsv->hsv.v;
    if (out_hsv->hsv.s == 0)
    {
        out_hsv->hsv.h = 0;
        return;
    }

    if (rgb_max == in_rgb->rgb.r)
        out_hsv->hsv.h = 0 + 43 * (in_rgb->rgb.g - in_rgb->rgb.b) / (rgb_max - rgb_min);
    else if (rgb_max == in_rgb->rgb.g)
        out_hsv->hsv.h = 85 + 43 * (in_rgb->rgb.b - in_rgb->rgb.r) / (rgb_max - rgb_min);
    else
        out_hsv->hsv.h = 171 + 43 * (in_rgb->rgb.r - in_rgb->rgb.g) / (rgb_max - rgb_min);

    if (out_hsv->hsv.h < 0)
        out_hsv->hsv.h = 255 + out_hsv->hsv.h;
   
}


static void estc_hsv_machine_calculate_rgb_values(ESTCHSVMachine* hsv_machine)
{
    hsv_to_rgb(&hsv_machine->led_color.hsv_components[0], &hsv_machine->pwm_values[1]);
    for (int i = 0; i < HSV_COMPONENTS; i++)
    {
        hsv_machine->pwm_values[i + 1] = hsv_machine->pwm_values[i + 1] * hsv_machine->pwm_max_value / MAX_COMPONENT_VALUES[i];
    }
}

/**
 * Set new state
**/
void estc_hsv_machine_next_state(ESTCHSVMachine* hsv_machine)
{
    ESTCTimeStamp current_time = estc_monotonic_time_get();
    estc_hsv_machine_calculate_mode_led_pwm(hsv_machine, current_time);
}

void estc_hsv_machine_increase_component(ESTCHSVMachine* hsv_machine)
{
    if (hsv_machine->mode == 0)
        return;

    uint32_t component_index = hsv_machine->mode - 1;
    int32_t * hsv_component = &hsv_machine->led_color.hsv_components[component_index];
    bool * component_increasing = &hsv_machine->hsv_components_increasing[component_index];

    ESTCTimeStamp current_time = estc_monotonic_time_get();
    uint32_t time_elapsed = estc_monotonic_time_diff(hsv_machine->component_increasing_step_timestamp,
            current_time);
    if (time_elapsed >= COMPONENT_INCREASING_PERIOD / MAX_COMPONENT_VALUES[component_index])
    {
        hsv_machine->component_increasing_step_timestamp = current_time;
        if (*component_increasing)
        {
            (*hsv_component)++;
            if (*hsv_component == MAX_COMPONENT_VALUES[component_index])
            {
                *component_increasing = false;
            }
        } 
        else 
        {
            (*hsv_component)--;
            if (*hsv_component == 0 )
            {
                *component_increasing = true;
            }            
        }
        NRF_LOG_INFO("HSV %d %d %d", hsv_machine->led_color.hsv.h, hsv_machine->led_color.hsv.s,
                hsv_machine->led_color.hsv.v);
        estc_hsv_machine_calculate_rgb_values(hsv_machine);
    }
}

uint32_t estc_hsv_machine_get_led_pwm(ESTCHSVMachine* hsv_machine, uint32_t led_number)
{
    assert(led_number < HSV_MACHINE_LEDS);
    return hsv_machine->pwm_values[led_number];
}

HSVColor estc_hsv_machine_get_components(ESTCHSVMachine* hsv_machine)
{
    return hsv_machine->led_color;
}

void estc_hsv_machine_set_components(ESTCHSVMachine* hsv_machine, const HSVColor * led_color)
{
    hsv_machine->led_color = *led_color;
    for (int i = 0; i < HSV_COMPONENTS; i++)
    {
        hsv_machine->hsv_components_increasing[i] = !(led_color->hsv_components[i] >= MAX_COMPONENT_VALUES[i]);
    }
    estc_hsv_machine_calculate_rgb_values(hsv_machine);    
}

void estc_hsv_machine_set_components_rgb(ESTCHSVMachine* hsv_machine, const RGBColor * rgb_color)
{
    rgb_to_hsv(rgb_color, &hsv_machine->led_color);
    for (int i = 0; i < HSV_COMPONENTS; i++)
    {
        hsv_machine->hsv_components_increasing[i] = !(hsv_machine->led_color.hsv_components[i] >= MAX_COMPONENT_VALUES[i]);
    }
    estc_hsv_machine_calculate_rgb_values(hsv_machine);    
}