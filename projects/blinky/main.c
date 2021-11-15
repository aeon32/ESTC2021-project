
#include <stdbool.h>
#include <stdint.h>
#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_log_ctrl.h>
#include <nrf_log_default_backends.h>
#include <nrf_log_backend_usb.h>
#include <nrfx_systick.h>
#include <app_usbd.h>

#include <boards.h>


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
   };

   nrf_gpio_cfg(ESTC_BUTTON_PIN, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);   

};

void switch_led_on (int led_number)
{

    uint32_t pin_number = ESTC_LEDS_PINS [led_number];
    nrf_gpio_pin_clear(pin_number);

};

void switch_led_off (int led_number)
{

    uint32_t pin_number = ESTC_LEDS_PINS [led_number];
    nrf_gpio_pin_set(pin_number);

};

void led_toggle (int led_number)
{

    uint32_t pin_number = ESTC_LEDS_PINS [led_number];
    nrf_gpio_pin_toggle(pin_number);

};


bool button_is_pressed()
{
   return nrf_gpio_pin_read(ESTC_BUTTON_PIN) == 0; 
};




typedef struct 
{
    //timestamp when pwm period was started. LED had to be switched on.
    nrfx_systick_state_t  pwm_period_start_timestamp;
    //from 0 to PWM_VALUE_MAX
    uint32_t pwm_value;    
    uint32_t new_pwm_value;
    bool led_switched_on;

    nrfx_systick_state_t  current_pwm_value_start_timestamp;
    bool brightness_increasing;
    uint32_t sequence_step;
 
} BlinkyMachineState;

void blinky_machine_state_init(BlinkyMachineState * blinkyMachineState)
{
    
    nrfx_systick_get(&blinkyMachineState->pwm_period_start_timestamp);
    nrfx_systick_get(&blinkyMachineState->current_pwm_value_start_timestamp);
    blinkyMachineState->pwm_value = 0;
    blinkyMachineState->new_pwm_value = 0;
    blinkyMachineState->sequence_step = 0;
    blinkyMachineState->led_switched_on = true;
    blinkyMachineState->brightness_increasing = true;
};


void blinky_machine_state_next(BlinkyMachineState * blinky_machine_state)
{
    
    uint32_t current_led = ESTC_BLINK_SEQUENCE[blinky_machine_state->sequence_step];


    if (nrfx_systick_test(&blinky_machine_state->current_pwm_value_start_timestamp, BLINK_PERIOD / PWM_VALUE_MAX/ 2))
    {
        nrfx_systick_get(&blinky_machine_state->current_pwm_value_start_timestamp);
        
        if (blinky_machine_state->brightness_increasing)
        {
            if (blinky_machine_state->new_pwm_value == PWM_VALUE_MAX)
            {
                //now led is fading..
                blinky_machine_state->brightness_increasing = false;
            } else {
                blinky_machine_state->new_pwm_value++;
                NRF_LOG_INFO("PWM_VALUE %d", blinky_machine_state->new_pwm_value);

            };

        };

        if (!blinky_machine_state->brightness_increasing)
        {
            if (blinky_machine_state->new_pwm_value == 0)
            {
                //led have done his working cycle, go to the next
                NRF_LOG_INFO("Led %d toggled", current_led);
                blinky_machine_state->sequence_step++;
                if (blinky_machine_state->sequence_step == SEQUENCE_SIZE)
                {
                  blinky_machine_state->sequence_step = 0;
                };
                blinky_machine_state->brightness_increasing = true;


            } else {
               blinky_machine_state->new_pwm_value--;
               NRF_LOG_INFO("PWM_VALUE %d", blinky_machine_state->new_pwm_value);

            };


        };     

    };
    
};


void blinky_machine_state_pwm_handle(BlinkyMachineState * blinky_machine_state)
{
    
    
    uint32_t current_led = ESTC_BLINK_SEQUENCE[blinky_machine_state->sequence_step];
    
    if (blinky_machine_state->led_switched_on)
    {
        uint32_t led_on_timeout = blinky_machine_state->pwm_value * PWM_PERIOD / PWM_VALUE_MAX;
        if(nrfx_systick_test(&blinky_machine_state->pwm_period_start_timestamp, led_on_timeout))
        {
            switch_led_off(current_led);
            blinky_machine_state->led_switched_on = false;
        };

    } else 
    {
        uint32_t led_off_timeout = PWM_PERIOD;
        if(nrfx_systick_test(&blinky_machine_state->pwm_period_start_timestamp, led_off_timeout))
        {
            //next pwm period started..
            nrfx_systick_get(&blinky_machine_state->pwm_period_start_timestamp);
            blinky_machine_state->pwm_value = blinky_machine_state->new_pwm_value;

            if (blinky_machine_state->pwm_value > 0)
                switch_led_on(current_led);
            blinky_machine_state->led_switched_on = true;
            
        };        


    };
};

void log_init()
{

    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();

};

/**
 * @brief Function for application main entry.
 */
int main(void)
{

    
    /* Configure board. */
    configure_gpio();
    log_init();
    nrfx_systick_init();

    BlinkyMachineState  blinkyMachineState;

    blinky_machine_state_init(&blinkyMachineState);
    /* Toggle LEDs. */

    NRF_LOG_INFO("Entering main loop");


    while (true)
    {
        if (button_is_pressed())
        {
            blinky_machine_state_next( &blinkyMachineState);

        };
        blinky_machine_state_pwm_handle(&blinkyMachineState);
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();

    };
};

/**
 *@}
 **/
