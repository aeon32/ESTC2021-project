#ifndef ESTC_BUTTON_H
#define ESTC_BUTTON_H

#include <nrfx_systick.h>

typedef enum 
{
   ESTC_BUTTON_NO_DOUBLE_CLICK,
   ESTC_BUTTON_FIRST_CLICK,
   ESTC_BUTTON_SECOND_CLICK

} ESTC_BUTTON_DOUBLECLICK_STATE;

typedef struct 
{

    ESTC_BUTTON_DOUBLECLICK_STATE double_click;
    nrfx_systick_state_t  first_click_time_stamp;
 
} ESTCButton;


//Timeout between two clicks in doubleclick, usec
#define ESTC_BUTTON_DOUBLECLICK_TIMEOUT 500000

/**
 * Initialization
 * 
**/ 
void estc_button_init(ESTCButton * button);

/**
 * Process button click.
 * Should be placed in the interrupt handler, or called via polling.
**/
void estc_button_process_click(ESTCButton * button);

/**
 * Will return true if button have been doubleclicked.
 * Clear doubleclicked flag
**/
bool estc_button_have_been_doubleclicked(ESTCButton * button);

#endif