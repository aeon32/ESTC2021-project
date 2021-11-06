
#include <stdbool.h>
#include <stdint.h>
#include <nrf_delay.h>
#include <boards.h>

//total leds number
#define ESTC_LEDS_NUMBER 4

//pin numbers. Constants are defined in nRF52840 Dongle UserGuide
static const uint8_t ESTC_LEDS_PINS[ESTC_LEDS_NUMBER] = 
   {
    NRF_GPIO_PIN_MAP(0,6),
    NRF_GPIO_PIN_MAP(0,8),
    NRF_GPIO_PIN_MAP(1,9),
    NRF_GPIO_PIN_MAP(0,12)
   };

static const int ESTC_LED_BLINK_TIMES[ESTC_LEDS_NUMBER] = {6, 5, 9, 9}; //equal to device id
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

};

void switch_led_on (int led_number)
{

    int pin_number = ESTC_LEDS_PINS [led_number];
    nrf_gpio_pin_clear(pin_number);

};

void switch_led_off (int led_number)
{

    int pin_number = ESTC_LEDS_PINS [led_number];
    nrf_gpio_pin_set(pin_number);

};


/**
 * @brief Function for application main entry.
 */
int main(void)
{

    
    /* Configure board. */
    configure_gpio();

    int current_led = 0;
    /* Toggle LEDs. */
    while (true)
    {


        for (int i = 0; i < ESTC_LED_BLINK_TIMES[current_led]; i++)
        {
            
            switch_led_on(current_led);
            nrf_delay_ms(ESTC_LED_ON_TIME);
            switch_led_off(current_led);
            nrf_delay_ms(ESTC_LED_ON_TIME);
        }

        current_led++;
        if (current_led == ESTC_LEDS_NUMBER)    
            current_led = 0;
        
    }
}

/**
 *@}
 **/
