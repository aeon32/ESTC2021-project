
#include <stdbool.h>
#include <stdint.h>
#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_log_ctrl.h>
#include <nrf_log_default_backends.h>
#include <nrf_log_backend_usb.h>
#include <nrfx_gpiote.h>
#include <nrfx_systick.h>
#include <app_usbd.h>
#include <app_timer.h>
#include <sdk_errors.h>

#include <boards.h>

#include "estc_pwm.h"
#include "estc_blinky_machine.h"
#include "estc_button.h"
#include "estc_monotonic_time.h"


//total leds number
#define ESTC_LEDS_NUMBER 4



//pin numbers for leds. Constants are defined in nRF52840 Dongle UserGuide
static const uint8_t ESTC_LEDS_PINS[ESTC_LEDS_NUMBER] = 
{
    NRF_GPIO_PIN_MAP(0,6),
    NRF_GPIO_PIN_MAP(0,8),
    NRF_GPIO_PIN_MAP(1,9),
    NRF_GPIO_PIN_MAP(0,12)
};

//pin numbers for button. Constants are defined in nRF52840 Dongle UserGuide
static const uint8_t ESTC_BUTTON_PIN = NRF_GPIO_PIN_MAP(1,6);


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

//rtc timer period in us, approximately
//We choose this value trying to minimize rounding error of RTC_FREQUENCY_DIVIDER calculation.
static const int RTC_PERIOD = 5188;

//clock divider == APP_TIMER_CLOCK_FREQ / ( 1000000 usec / RTC_PERIOD usec);
static const int RTC_FREQUENCY_DIVIDER =  RTC_PERIOD*APP_TIMER_CLOCK_FREQ/1000000; 

void configure_gpio()
{

   for ( int i = 0; i < ESTC_LEDS_NUMBER; i++)
   {
       int pin_number = ESTC_LEDS_PINS [i];
       //configure all led pins for output
       nrf_gpio_cfg(pin_number, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT,
                    NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE
                   );
       //switch off led by setting high level
       nrf_gpio_pin_write(pin_number, 1);
   }

   //nrf_gpio_cfg(ESTC_BUTTON_PIN, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT,
   //              NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);   

}


bool button_is_pressed()
{
   return nrf_gpio_pin_read(ESTC_BUTTON_PIN) == 0; 
}


void log_init()
{

    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();

}

void led_toggle (int led_number)
{

    uint32_t pin_number = ESTC_LEDS_PINS [led_number];
    nrf_gpio_pin_toggle(pin_number);

}

typedef struct _Application
{
    ESTCButton button;
    ESTCBlinkyMachine blinky_machine;
    ESTCPWM pwm_leds[ESTC_LEDS_NUMBER];
    bool smooth_blinking;

} Application;
static Application app;


void button_on_doubleclick_handler(void * user_data)
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
   //some kind of a critical section
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


void button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{

    application_lock(&app);
    estc_button_process_click(&app.button);
    application_unlock(&app);

    NRF_LOG_INFO("Polarity %d", action);

}




void gpiote_init()
{
    nrfx_err_t err = nrfx_gpiote_init();
    APP_ERROR_CHECK(err);

    nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err = nrfx_gpiote_in_init(ESTC_BUTTON_PIN, &in_config, button_handler);
    APP_ERROR_CHECK(err);

    nrfx_gpiote_in_event_enable (ESTC_BUTTON_PIN, true);
  
}

void rtc_handler (void *p_context)
{
   
    estc_monotonic_time_update(RTC_PERIOD);  

}



void rtc_init()
{
    ret_code_t err  = app_timer_init();
    APP_ERROR_CHECK(err);

    APP_TIMER_DEF(default_timer_id);

    err = app_timer_create(&default_timer_id, APP_TIMER_MODE_REPEATED, rtc_handler );
    err = app_timer_start(default_timer_id, RTC_FREQUENCY_DIVIDER, NULL );    

}



int main(void)
{

    
    /* Configure board. */
    nrfx_systick_init();
    application_init(&app);

    configure_gpio();
    log_init();
    rtc_init();
    gpiote_init();

    



    NRF_LOG_INFO("Entering main loop");
    while (true)
    {
        application_lock(&app);
        application_next_tick(&app);
        application_unlock(&app);

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();

    }
}

/**
 *@}
 **/
