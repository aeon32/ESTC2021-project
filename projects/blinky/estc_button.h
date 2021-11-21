#ifndef ESTC_BUTTON_H
#define ESTC_BUTTON_H

#include "estc_monotonic_time.h"

typedef enum 
{
   ESTC_BUTTON_RELEASED_STATE,
   ESTC_BUTTON_FIRST_CLICK,
   ESTC_BUTTON_LONG_PRESS,
   ESTC_BUTTON_WAITING_FOR_SECOND_CLICK,
   ESTC_BUTTON_SECOND_CLICK

} ESTC_BUTTON_DOUBLECLICK_STATE;



typedef void (*ESTCButtonEventHandler) (void * user_data);

typedef struct 
{

    ESTC_BUTTON_DOUBLECLICK_STATE double_click;
    ESTCTimeStamp  first_click_time_stamp;

    ESTCTimeStamp pressed_time_stamp;
    ESTCTimeStamp filtered_pressed_time_stamp;
    ESTCTimeStamp released_time_stamp;

    ESTCButtonEventHandler double_click_handler;
    ESTCButtonEventHandler long_press_handler;
    bool pressed;
    bool filtered_pressed; //antibounced button pressed flag
    void * user_data;
 
} ESTCButton;


//Timeout between two clicks in doubleclick, usec
#define ESTC_BUTTON_DOUBLECLICK_TIMEOUT 1000000
//Timeout between first click and on_press event firing
#define ESTC_BUTTON_PRESS_TIMEOUT 1500000
//Anti-contact bounce guard timeout
#define ESTC_BUTTON_BOUNCE_GUARD_TIMEOUT 15000

/**
 * Initialization
 * 
**/ 
void estc_button_init(ESTCButton * button, ESTCButtonEventHandler double_click_handler, 
                      ESTCButtonEventHandler long_press_handler, void * user_data);

/**
 * Process button press
 * 
**/
void estc_button_process_press(ESTCButton * button);

/**
 * Process button release
 * 
**/
void estc_button_process_release(ESTCButton * button);


/**
 * Update button state by time.
 * Should be called from loop or timer interrupt
 * 
**/
void estc_button_process_update(ESTCButton * button);

#endif