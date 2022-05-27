#include "application.h"
#include "estc_strutils.h"

#include <nrf_log.h>
#include <string.h>
#include <stdlib.h>

enum STORAGE_DATA_TYPES
{
    STORAGE_HSV_VALUES = 0
};

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


static bool application_load_hsv_from_flash(Application * app, HSVColor * out_color)
{
    bool res = false;

    return res;
}

static bool application_save_hsv_to_flash(Application * app, HSVColor * color)
{
    bool res = false;
    return res;
}

static void application_update_color(Application * app)
{
    app->color = estc_hsv_machine_get_components(&app->hsv_machine);

    if (app->color_handler)
    {
        app->color_handler(app, app->color_change_handler_user_data);
    }

    application_save_hsv_to_flash(app, &app->color);
    application_load_hsv_from_flash(app, &app->color);
    
}

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

static void button_on_release_after_longpress_handler(void * user_data)
{
    application_update_color((Application *) user_data);

}

static void pwm_handler(nrfx_pwm_evt_type_t event_type)
{

    NRFX_CRITICAL_SECTION_ENTER();

    app.duty_cycle_values.channel_0 = estc_hsv_machine_get_led_pwm(&app.hsv_machine, 0);
    app.duty_cycle_values.channel_1 = estc_hsv_machine_get_led_pwm(&app.hsv_machine, 1);
    app.duty_cycle_values.channel_2 = estc_hsv_machine_get_led_pwm(&app.hsv_machine, 2);
    app.duty_cycle_values.channel_3 = estc_hsv_machine_get_led_pwm(&app.hsv_machine, 3);

    NRFX_CRITICAL_SECTION_EXIT();
}



static void hsv_machine_toggle_mode_handler(ESTCHSVMachineMode new_mode, void * user_data)
{
    //if (new_mode != ESTCHSV_NO_INPUT)
    //    return;
    
    application_update_color((Application *) user_data);
}

#if defined(CLI_SUPPORT) && CLI_SUPPORT
static const char * TERMINAL_COMMAND_DELIMITER = " \t";

static void application_cli_write(Application * app, const char * data, size_t data_size)
{
    estc_uart_write(data, data_size);
}

static bool terminal_command_handle_rgb(char ** strtok_context, Application * app)
{
    NRF_LOG_INFO("RGB command handler");
    bool command_ok = true;
    RGBColor led_color;
    int i = 0;
    for (; command_ok && i < RGB_COMPONENTS; i++)
    {
        char * token = estc_strtok_r(NULL, TERMINAL_COMMAND_DELIMITER, strtok_context);
        command_ok = (token != NULL);
        long int comp_value;
        if (command_ok)
        {   
            char * endptr;
            comp_value = strtol(token, &endptr, 10);
            command_ok = (*endptr == '\0');
            NRF_LOG_INFO("RGB command strtol %d %d", comp_value, command_ok);        
        }
        if (command_ok)
        {
            switch (i)
            {
                case 0:
                   command_ok = (comp_value >=0 ) && (comp_value <=255);
                break;
                case 1:
                   command_ok = (comp_value >=0 ) && (comp_value <=255);
                break;
                case 2:
                   command_ok = (comp_value >=0 ) && (comp_value <=255);
                break;                                                
            }
        }
        if (command_ok)
            led_color.rgb_components[i] = comp_value;
    }
    command_ok = command_ok && (i == RGB_COMPONENTS); //all components is set

    if (command_ok)
    {
        estc_hsv_machine_set_components_rgb(&app->hsv_machine, &led_color);
        app->color = estc_hsv_machine_get_components(&app->hsv_machine);
        if (app->color_handler)
        {
            app->color_handler(app, app->color_change_handler_user_data);
        }
        
        application_save_hsv_to_flash(app, &app->color);
        const char * save_ok_string = "Successful.\r\n";
        application_cli_write(app, save_ok_string, strlen( save_ok_string));
    }
    return command_ok;
}

static bool terminal_command_handle_hsv(char ** strtok_context, Application * app)
{
    NRF_LOG_INFO("HSV command handler");
    bool command_ok = true;
    HSVColor led_color;
    int i = 0;
    for (; command_ok && i < HSV_COMPONENTS; i++)
    {
        char * token = estc_strtok_r(NULL, TERMINAL_COMMAND_DELIMITER, strtok_context);
        command_ok = (token != NULL);
        long int comp_value;
        if (command_ok)
        {   
            char * endptr;
            comp_value = strtol(token, &endptr, 10);
            command_ok = (*endptr == '\0');
            NRF_LOG_INFO("HSV command strtol %d %d", comp_value, command_ok);        
        }
        if (command_ok)
        {
            switch (i)
            {
                case 0:
                   command_ok = (comp_value >=0 ) && (comp_value <=360);
                   comp_value = comp_value * 255 / 360;  //normalize to estc_hsv_machine standard
                break;
                case 1:
                   command_ok = (comp_value >=0 ) && (comp_value <=100);
                   comp_value = comp_value * 255 / 100;  //normalize to estc_hsv_machine standard
                break;
                case 2:
                   command_ok = (comp_value >=0 ) && (comp_value <=100);
                   comp_value = comp_value * 255 / 100;  //normalize to estc_hsv_machine standard
                break;                                                
            }
        }
        if (command_ok)
            led_color.hsv_components[i] = comp_value;
    }
    command_ok = command_ok && (i == HSV_COMPONENTS); //all components is set

    if (command_ok)
    {
        estc_hsv_machine_set_components(&app->hsv_machine, &led_color);
        app->color = led_color;
        if (app->color_handler)
        {
            app->color_handler(app, app->color_change_handler_user_data);
        }

        application_save_hsv_to_flash(app, &led_color);
        const char * save_ok_string = "Successful.\r\n";
        application_cli_write(app, save_ok_string, strlen( save_ok_string));
        UNUSED_VARIABLE(led_color);
    }
    return command_ok;
}

static void terminal_command_handler(char * command, void * user_data)
{
    Application * app = (Application *) user_data;
    char * context = NULL;
    char * token = estc_strtok_r(command, TERMINAL_COMMAND_DELIMITER, &context);
    NRF_LOG_INFO("Command %s", command);
    bool command_ok = true;
    if (token)
    {
        NRF_LOG_INFO("Token %s", token);
        if (strcmp(token,"RGB") == 0)
        {
            command_ok = terminal_command_handle_rgb(&context, app);
        } 
        else if (strcmp(token, "HSV") == 0)
        {
            command_ok = terminal_command_handle_hsv(&context, app);
        } 
        else 
        {
            command_ok = false;
        }
    } 
    if (!command_ok)
    {
        const char * help_string = "Unknown command.\r\nUsage:\r\n"
        "RGB <red:0-255> <green:0-255> <blue:0-255>\r\n-or-\r\nHSV <hur:0-360> <saturation:0-100> <value:0-100>\r\n";
        application_cli_write(app, help_string, strlen(help_string));
    }
}
#endif //CLI_SUPPORT

void application_init(Application* app)
{
    memset(app, 0, sizeof(Application));
    HSVColor led_color = {.hsv_components = {0, 255, 255}};
    app->color = led_color;

    application_load_hsv_from_flash(app, &app->color);
    
    estc_button_init(&app->button, button_on_doubleclick_handler, button_on_longpress_handler, button_on_release_after_longpress_handler, app);
    estc_hsv_machine_init(&app->hsv_machine, &app->color, PWM_VALUE_MAX, 
        hsv_machine_toggle_mode_handler, app );

#if defined(CLI_SUPPORT) && CLI_SUPPORT
    estc_uart_term_init(terminal_command_handler, app);
#endif        
    
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
#ifdef CLI_SUPPORT
    estc_uart_term_process_events();
#endif    
}

void application_process_press(Application* app)
{
    estc_button_process_press(&app->button);
}

void application_process_release(Application* app)
{
    estc_button_process_release(&app->button);
}

void application_get_color_as_text(Application * app, char * out_buffer, size_t * out_len)
{
   long int norm_h = app->color.hsv.h*360/255;
   long int norm_s = app->color.hsv.s*100/255;
   long int norm_v = app->color.hsv.v*100/255;
   *out_len = sprintf(out_buffer, "HSV %ld %ld %ld", 
            norm_h, norm_s, norm_v );
}

 void application_set_color_change_handler(Application * app, ColorChangeHandler handler, void * color_change_handler_user_data)
 {
     app->color_handler = handler;
     app->color_change_handler_user_data = color_change_handler_user_data;
 }

Application app;
