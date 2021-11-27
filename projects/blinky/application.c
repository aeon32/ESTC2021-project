#include "application.h"

#include <nrf_log.h>

//pin numbers for leds. Constants are defined in nRF52840 Dongle UserGuide
const uint8_t ESTC_LEDS_PINS[ESTC_LEDS_NUMBER] =
{
    NRF_GPIO_PIN_MAP(0, 6),
    NRF_GPIO_PIN_MAP(0, 8),
    NRF_GPIO_PIN_MAP(1, 9),
    NRF_GPIO_PIN_MAP(0, 12)
};

//pin numbers for button. Constants are defined in nRF52840 Dongle UserGuide
const uint8_t ESTC_BUTTON_PIN = NRF_GPIO_PIN_MAP(1, 6);


//PWM FREQUENCY in herz
//static const int PWM_FREQUENCY = 1000;
//PWM_PERIOD in usec
//static const int PWM_PERIOD = 1000000 / PWM_FREQUENCY;
//frequency of blink

//max pwm value (equal to 100%)
static const int PWM_VALUE_MAX = 50;

static nrfx_pwm_t PWM_INSTANCE = NRFX_PWM_INSTANCE(0);

static void button_on_doubleclick_handler(void* user_data)
{
    //C-style leg shooting )
    Application* app = (Application*)user_data;
    estc_hsv_machine_switch_mode(&app->hsv_machine);
}

static void button_on_longpress_handler(void* user_data)
{
    Application* app = (Application*)user_data;
    estc_hsv_machine_increase_component(&app->hsv_machine);
}

static void pwm_handler(nrfx_pwm_evt_type_t event_type)
{
    application_lock(&app);

    app.duty_cycle_values.channel_0 = estc_hsv_machine_get_led_pwm(&app.hsv_machine, 0);
    app.duty_cycle_values.channel_1 = estc_hsv_machine_get_led_pwm(&app.hsv_machine, 1);
    app.duty_cycle_values.channel_2 = estc_hsv_machine_get_led_pwm(&app.hsv_machine, 2);
    app.duty_cycle_values.channel_3 = estc_hsv_machine_get_led_pwm(&app.hsv_machine, 3);

    application_unlock(&app);
}

void application_init(Application* app)
{
    memset(app, 0, sizeof(Application));
    estc_button_init(&app->button, button_on_doubleclick_handler, button_on_longpress_handler, app);
    estc_hsv_machine_init(&app->hsv_machine, PWM_VALUE_MAX);

    app->sequence.values.p_individual = &app->duty_cycle_values;
    app->sequence.length = NRF_PWM_VALUES_LENGTH(app->duty_cycle_values);
    app->sequence.repeats = 0;
    app->sequence.end_delay = 0;

    nrfx_err_t err;

    nrfx_pwm_config_t config = NRFX_PWM_DEFAULT_CONFIG;
    config.output_pins[0] = ESTC_LEDS_PINS[0];
    config.output_pins[1] = ESTC_LEDS_PINS[1];
    config.output_pins[2] = ESTC_LEDS_PINS[2];
    config.output_pins[3] = ESTC_LEDS_PINS[3];
    config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    config.top_value = PWM_VALUE_MAX;

    err = nrfx_pwm_init(
            &PWM_INSTANCE,
            &config,
            pwm_handler
    );
    APP_ERROR_CHECK(err);
    nrfx_pwm_simple_playback(&PWM_INSTANCE, &app->sequence, 1, NRFX_PWM_FLAG_LOOP);
}

void application_next_tick(Application* app)
{
    estc_button_process_update(&app->button);
    estc_hsv_machine_next_state(&app->hsv_machine);
}

void application_lock(Application* app)
{
    NRFX_CRITICAL_SECTION_ENTER();
}

void application_unlock(Application* app)
{
    NRFX_CRITICAL_SECTION_EXIT();
}

void application_process_press(Application* app)
{
    estc_button_process_press(&app->button);
}

void application_process_release(Application* app)
{
    estc_button_process_release(&app->button);
}

Application app;
