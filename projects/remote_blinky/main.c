
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
#include <nrf_pwr_mgmt.h>

#include <boards.h>

#include "estc_pwm.h"
#include "estc_blinky_machine.h"
#include "estc_button.h"
#include "estc_monotonic_time.h"
#include "estc_ble.h"
#include "estc_service.h"

#include "application.h"

//rtc timer period in us, approximately
//We choose this value trying to minimize rounding error of RTC_FREQUENCY_DIVIDER calculation.
static const int RTC_PERIOD = 5188;

//clock divider == ESTC_TIMER_CLOCK_FREQ / ( 1000000 usec / RTC_PERIOD usec);
static const int RTC_FREQUENCY_DIVIDER = RTC_PERIOD * APP_TIMER_CLOCK_FREQ / 1000000;

//BLE params
#define DEVICE_NAME                     "ESTColour"                             /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "NordicSemiconductor"                   /**< Manufacturer. Will be passed to Device Information Service. */

#define ESTC_BASE_UUID {0x57,0xB7, 0xA8, 0xBF, 0xB0, 0x78, 0x21, 0x43, 0xA6, 0x71, 0x16, 0x88, 0x12, 0x04, 0xA4, 0x64}
#define ESTC_SERVICE_UUID  0x1204 
#define ESTC_SERVICE_HSV_CHAR_UUID 0x1205

typedef struct 
{
    estc_ble_service_t base; 
    ble_gatts_char_handles_t color_char_handle;
} blinky_service_t;

blinky_service_t m_blinky_service;

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
    NRFX_CRITICAL_SECTION_ENTER();
   
    if (button_is_pressed())
    {
        application_process_press(&app);
    }
    else
    {
        application_process_release(&app);
    }
    NRFX_CRITICAL_SECTION_EXIT();
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
}

void rtc_init()
{
    ret_code_t err = app_timer_init();
    APP_ERROR_CHECK(err);

    APP_TIMER_DEF(default_timer_id);

    err = app_timer_create(&default_timer_id, APP_TIMER_MODE_REPEATED, rtc_handler);
    err = app_timer_start(default_timer_id, RTC_FREQUENCY_DIVIDER, NULL);
}

static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


static void blinky_service_init(estc_ble_t * estc_ble, Application * app)
{
   memset(&m_blinky_service, 0, sizeof(blinky_service_t));
   ble_uuid128_t  base_uuid128 = {ESTC_BASE_UUID};

   ret_code_t err_code = estc_ble_service_init(&m_blinky_service.base, estc_ble, &base_uuid128, ESTC_SERVICE_UUID );
   APP_ERROR_CHECK(err_code);

   char color_value[ESTC_HSV_COLOR_BUFFER_MAX_LEN];
   memset(color_value, ' ', sizeof(color_value));   //padding for better displaying in mobile app
   size_t outlen;
   application_get_color_as_text(app, color_value, &outlen );

   err_code = estc_ble_add_characteristic(&m_blinky_service.base, ESTC_SERVICE_HSV_CHAR_UUID, 
                                           "HSVColor ", (uint8_t *) color_value,
                                           ESTC_HSV_COLOR_BUFFER_MAX_LEN -1, ESTC_HSV_COLOR_BUFFER_MAX_LEN -1,
                                           ESTC_CHAR_READ | ESTC_CHAR_NOTIFY | ESTC_CHAR_TEXT_FORMAT, &m_blinky_service.color_char_handle);
   APP_ERROR_CHECK(err_code);
   //NRF_SDH_BLE_OBSERVER(m_connection_observer_observer, ESTC_BLE_OBSERVER_PRIO, connection_handler, &m_blinky_service);    
}

static void color_change_handler(Application * app, void * user_data)
{
    blinky_service_t * blinky_service = (blinky_service_t *) user_data;
    uint16_t connection_handle = estc_ble_connection_handle(blinky_service->base.estc_ble);
    NRF_LOG_INFO("color handler");

    if (connection_handle != BLE_CONN_HANDLE_INVALID)
    {
        char color_value[ESTC_HSV_COLOR_BUFFER_MAX_LEN];
        memset(color_value, ' ', sizeof(color_value));   //padding for better displaying in mobile app
        size_t outlen;
        application_get_color_as_text(app, color_value, &outlen );
        NRF_LOG_INFO("notify %s", color_value);
        estc_char_notify(connection_handle, &blinky_service->color_char_handle,
                            (uint8_t * ) color_value, ESTC_HSV_COLOR_BUFFER_MAX_LEN -1 ); 

    }
    
}

int main(void)
{
    /* Configure board. */
    nrfx_systick_init();
    log_init();
    application_init(&app);
    configure_gpio();
    rtc_init();
    gpiote_init();
    power_management_init();
    
    estc_ble_t * estc_ble = estc_ble_init(DEVICE_NAME, MANUFACTURER_NAME);  
    blinky_service_init(estc_ble, &app);

    application_set_color_change_handler(&app, color_change_handler, &m_blinky_service);

    NRF_LOG_INFO("Entering main loop");
    estc_ble_start(estc_ble);
    while (true)
    {
        NRFX_CRITICAL_SECTION_ENTER();
        application_next_tick(&app);
        NRFX_CRITICAL_SECTION_EXIT();   
        LOG_BACKEND_USB_PROCESS();
        if (NRF_LOG_PROCESS() == false)
        {
            nrf_pwr_mgmt_run();
        }
    }
}
