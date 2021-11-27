
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

#include "application.h"

//rtc timer period in us, approximately
//We choose this value trying to minimize rounding error of RTC_FREQUENCY_DIVIDER calculation.
static const int RTC_PERIOD = 5188;

//clock divider == ESTC_TIMER_CLOCK_FREQ / ( 1000000 usec / RTC_PERIOD usec);
static const int RTC_FREQUENCY_DIVIDER = RTC_PERIOD * APP_TIMER_CLOCK_FREQ / 1000000;

void configure_gpio()
{
    for (int i = 0; i < ESTC_LEDS_NUMBER; i++)
    {
        int pin_number = ESTC_LEDS_PINS[i];
        //configure all led pins for output
        nrf_gpio_cfg(pin_number, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT,
                NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE
        );
        //switch off led by setting high level
        nrf_gpio_pin_write(pin_number, 1);
    }
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

void led_toggle(int led_number)
{
    uint32_t pin_number = ESTC_LEDS_PINS[led_number];
    nrf_gpio_pin_toggle(pin_number);
}

void button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    application_lock(&app);
    if (button_is_pressed())
    {
        application_process_press(&app);
    }
    else
    {
        application_process_release(&app);
    }
    application_unlock(&app);
}

void gpiote_init()
{
    nrfx_err_t err = nrfx_gpiote_init();
    APP_ERROR_CHECK(err);

    nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err = nrfx_gpiote_in_init(ESTC_BUTTON_PIN, &in_config, button_handler);
    APP_ERROR_CHECK(err);

    nrfx_gpiote_in_event_enable(ESTC_BUTTON_PIN, true);
}

void rtc_handler(void* p_context)
{
    estc_monotonic_time_update(RTC_PERIOD);
    application_lock(&app);
    application_next_tick(&app);
    application_unlock(&app);
}

void rtc_init()
{
    ret_code_t err = app_timer_init();
    APP_ERROR_CHECK(err);

    APP_TIMER_DEF(default_timer_id);

    err = app_timer_create(&default_timer_id, APP_TIMER_MODE_REPEATED, rtc_handler);
    err = app_timer_start(default_timer_id, RTC_FREQUENCY_DIVIDER, NULL);
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
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
}
