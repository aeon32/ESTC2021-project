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
#include "estc_storage.h"
#include "estc_uart_term.h"

//total leds number
#define ESTC_LEDS_NUMBER 4

//consts, defined in application.c
extern const uint8_t ESTC_LEDS_PINS[ESTC_LEDS_NUMBER];

extern const uint8_t ESTC_BUTTON_PIN;

typedef struct _Application
{
    ESTCButton button;
    ESTCHSVMachine hsv_machine;
    ESTCStorage storage;
    //Duty cycle values for a sequence loaded in NRF_PWM_LOAD_INDIVIDUAL
    nrf_pwm_values_individual_t duty_cycle_values;
    //structure for defining a sequence of PWM duty cycles
    nrf_pwm_sequence_t sequence;

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
 *  Critical section for Application instance
**/
void application_lock(Application* app);
void application_unlock(Application* app);

#endif