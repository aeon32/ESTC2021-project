
#include <stdbool.h>
#include <stdint.h>
#include <nrf_delay.h>
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
static const int ESTC_LED_ON_TIME = 500;   //Time period of led switched on/off   

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
    uint32_t current_tick;
    uint32_t sequence_step;
    bool led_switched_on;
} BlinkyMachineState;

void blinky_machine_state_init(BlinkyMachineState * blinkyMachineState)
{
    blinkyMachineState->current_tick = 0;
    blinkyMachineState->sequence_step = 0;
    blinkyMachineState->led_switched_on = false;
};

void blinky_machine_state_next(BlinkyMachineState * blinkyMachineState)
{
    
    static const size_t SEQUENCE_SIZE = sizeof(ESTC_BLINK_SEQUENCE)/sizeof(ESTC_BLINK_SEQUENCE[0]);
    uint32_t current_led = ESTC_BLINK_SEQUENCE[blinkyMachineState->sequence_step];

    blinkyMachineState->current_tick++;
    

    if (blinkyMachineState->current_tick == ESTC_LED_ON_TIME)
    {
        
        blinkyMachineState->current_tick = 0;
        if (blinkyMachineState->led_switched_on)
        {
            blinkyMachineState->sequence_step++;
            if (blinkyMachineState->sequence_step == SEQUENCE_SIZE)
            {
               blinkyMachineState->sequence_step = 0;
            };

        };
        //we should toggle led
        led_toggle(current_led);
        blinkyMachineState->led_switched_on = !blinkyMachineState->led_switched_on;
    }

};


/**
 * @brief Function for application main entry.
 */
int main(void)
{

    
    /* Configure board. */
    configure_gpio();

    BlinkyMachineState  blinkyMachineState;

    blinky_machine_state_init(&blinkyMachineState);
    /* Toggle LEDs. */
    while (true)
    {
        if (button_is_pressed())
        {
            blinky_machine_state_next( &blinkyMachineState);

        };
        nrf_delay_ms(1);

    }
}

/**
 *@}
 **/
