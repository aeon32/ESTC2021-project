#include "application.h"


//pin numbers for leds. Constants are defined in nRF52840 Dongle UserGuide
const uint8_t ESTC_LEDS_PINS[ESTC_LEDS_NUMBER] = 
{
    NRF_GPIO_PIN_MAP(0,6),
    NRF_GPIO_PIN_MAP(0,8),
    NRF_GPIO_PIN_MAP(1,9),
    NRF_GPIO_PIN_MAP(0,12)
};

//pin numbers for button. Constants are defined in nRF52840 Dongle UserGuide
const uint8_t ESTC_BUTTON_PIN = NRF_GPIO_PIN_MAP(1,6);


//we encode blinking sequence as array of led number (equal to (6, 5, 9, 9) - device id)
static const uint32_t ESTC_BLINK_SEQUENCE[] = {
    0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3 

};

static const size_t SEQUENCE_SIZE = sizeof(ESTC_BLINK_SEQUENCE)/sizeof(ESTC_BLINK_SEQUENCE[0]);

//PWM FREQUENCY in herz
static const int PWM_FREQUENCY = 1000;
//PWM_PERIOD in usec
static const int PWM_PERIOD = 1000000 / PWM_FREQUENCY;
//frequency of blink
static const int BLINK_FREQUENCY = 2;
//period of blink 
static const int BLINK_PERIOD = 1000000 / BLINK_FREQUENCY;
//max pwm value (equal to 100%)
static const int PWM_VALUE_MAX = 50; 



static void button_on_doubleclick_handler(void * user_data)
{  
    //C-style leg shooting )
    Application * app = (Application *) user_data;
    
    app->smooth_blinking = !app->smooth_blinking;

}



void application_init(Application * app)
{
    estc_button_init(&app->button, button_on_doubleclick_handler, NULL, app);
    estc_blinky_machine_init(&app->blinky_machine, ESTC_BLINK_SEQUENCE, SEQUENCE_SIZE, BLINK_PERIOD, PWM_VALUE_MAX);

    
    for (int i = 0; i < ESTC_LEDS_NUMBER; i++)
    {
        estc_pwm_init(&app->pwm_leds[i], ESTC_LEDS_PINS[i], true, PWM_PERIOD, PWM_VALUE_MAX);

    }    
}

void application_next_tick(Application * app)
{
    if (app->smooth_blinking)
    {
        estc_blinky_machine_next_state(&app->blinky_machine);
        for (int i = 0; i < ESTC_LEDS_NUMBER; i++)
        {
            uint32_t new_led_pwm = estc_blinky_machine_get_led_pwm(&app->blinky_machine, i);
            estc_pwm_set_value(&app->pwm_leds[i], new_led_pwm);
        }

    }        
   
    for (int i = 0; i < ESTC_LEDS_NUMBER; i++)
    {
       estc_pwm_handle(&app->pwm_leds[i]);
    }
}

void application_lock(Application * app)
{
    NRFX_CRITICAL_SECTION_ENTER();

}

void application_unlock(Application * app)
{
    NRFX_CRITICAL_SECTION_EXIT();

}

void application_process_click(Application * app)
{
   estc_button_process_click(&app->button);
}

Application app;
