#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdbool.h>
#include <stdint.h>
#include <boards.h>
#include <app_timer.h>
#include <nrfx_pwm.h>

#include "estc_pwm.h"
#include "estc_hsv_machine.h"
#include "estc_button.h"
#include "estc_monotonic_time.h"
#include "estc_uart_term.h"
#include "estc_storage.h"

//total leds number
#define ESTC_LEDS_NUMBER 4

//buffsize, enough to store text representation of any cli command  (without trailing \0)
#define ESTC_CLI_COMMAND_MAX_LEN 16

//consts, defined in application.c
extern const uint8_t ESTC_LEDS_PINS[ESTC_LEDS_NUMBER];

extern const uint8_t ESTC_BUTTON_PIN;

typedef struct _Application Application;

typedef void (* ColorChangeHandler)(Application * app, void* user_data);

typedef struct _Application
{
    ESTCButton button;
    ESTCHSVMachine hsv_machine;
    ESTCStorage storage;
    //Duty cycle values for a sequence loaded in NRF_PWM_LOAD_INDIVIDUAL
    nrf_pwm_values_individual_t duty_cycle_values;
    //structure for defining a sequence of PWM duty cycles
    nrf_pwm_sequence_t sequence;
    //current color
    HSVColor color;
    //handler for changing color event    
    ColorChangeHandler color_handler;
    void * color_change_handler_user_data;
} Application;

//defined in application.c
extern Application app;

/**
 *  Structure init
**/
void application_init(Application* app);

/**
 *  Update scene, according to current intenal state
**/
void application_next_tick(Application* app);

/**
 * 
**/
void application_process_press(Application* app);
void application_process_release(Application* app);

/**
 * @brief Returns current color as text
 * @param out_buffer should have size at least ESTC_HSV_COLOR_BUFFER_MAX_LEN
**/
void application_get_color_as_text(Application * app, char * out_buffer, size_t * out_len);

/**
 * @brief Set handler on color change event
 * 
**/
void application_set_color_change_handler(Application * app, ColorChangeHandler handler, void * color_change_handler_user_data);

/**
 * @brief Handle cli command
 * 
 */
void application_handle_cli_command(Application * app, char * command, bool silent);

#endif