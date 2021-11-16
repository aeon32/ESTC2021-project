
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

#include <boards.h>

#include "estc_pwm.h"
#include "estc_blinky_machine.h"
#include "estc_button.h"


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


ESTCButton button;


void button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{

    estc_button_process_click(&button);

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



int main1()
{
    configure_gpio();
    estc_button_init(&button);
    log_init();
    nrfx_systick_init();
    gpiote_init();

    NRF_LOG_INFO("Entering main loop");
    while (true)
    {
     
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();

    }


}

int main(void)
{

    
    /* Configure board. */
    configure_gpio();
    estc_button_init(&button);
    log_init();
    nrfx_systick_init();
    gpiote_init();


    ESTCBlinkyMachine blinky_machine;
    estc_blinky_machine_init(&blinky_machine, ESTC_BLINK_SEQUENCE, SEQUENCE_SIZE, BLINK_PERIOD, PWM_VALUE_MAX);


    ESTCPWM pwm_leds[ESTC_LEDS_NUMBER];
    for (int i = 0; i < ESTC_LEDS_NUMBER; i++)
    {
        estc_pwm_init(&pwm_leds[i], ESTC_LEDS_PINS[i], true, PWM_PERIOD, PWM_VALUE_MAX);

    }

    bool smooth_blinking = false;


    NRF_LOG_INFO("Entering main loop");
    while (true)
    {
        //some kind of a critical section
        nrfx_gpiote_in_event_enable (ESTC_BUTTON_PIN, false);
        
        if (estc_button_have_been_doubleclicked(&button))
            smooth_blinking = ! smooth_blinking;

        nrfx_gpiote_in_event_enable (ESTC_BUTTON_PIN, true);

        if (smooth_blinking)
        {
            estc_blinky_machine_next_state(&blinky_machine);
            for (int i = 0; i < ESTC_LEDS_NUMBER; i++)
            {
                uint32_t new_led_pwm = estc_blinky_machine_get_led_pwm(&blinky_machine, i);
                estc_pwm_set_value(&pwm_leds[i], new_led_pwm);
            }

        }        
       
        for (int i = 0; i < ESTC_LEDS_NUMBER; i++)
        {
           estc_pwm_handle(&pwm_leds[i]);
        }

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();

    }
}

/**
 *@}
 **/
