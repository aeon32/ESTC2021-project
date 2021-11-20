#ifndef ESTC_BUTTON_H
#define ESTC_BUTTON_H

#include "estc_monotonic_time.h"

typedef enum 
{
   ESTC_BUTTON_NO_DOUBLE_CLICK,
   ESTC_BUTTON_FIRST_CLICK,
   ESTC_BUTTON_SECOND_CLICK

} ESTC_BUTTON_DOUBLECLICK_STATE;



typedef void (*ESTCButtonEventHandler) (void * user_data);

typedef struct 
{

    ESTC_BUTTON_DOUBLECLICK_STATE double_click;
    ESTCTimeStamp  first_click_time_stamp;

    ESTCButtonEventHandler double_click_handler;
    ESTCButtonEventHandler long_press_handler;
    void * user_data;
 
} ESTCButton;


//Timeout between two clicks in doubleclick, usec
#define ESTC_BUTTON_DOUBLECLICK_TIMEOUT 500000

/**
 * Initialization
 * 
**/ 
void estc_button_init(ESTCButton * button, ESTCButtonEventHandler double_click_handler, 
                      ESTCButtonEventHandler long_press_handler, void * user_data);

/**
 * Process button click.
 * Should be placed in the interrupt handler, or called via polling.
**/
void estc_button_process_click(ESTCButton * button);


#endif