#include "estc_button.h"
#include <nrf_log.h>
#include <boards.h>




void estc_button_init(ESTCButton * button, ESTCButtonEventHandler double_click_handler, 
                      ESTCButtonEventHandler long_press_handler, void * user_data)
{
    button->double_click = ESTC_BUTTON_RELEASED_STATE;   
    button->long_press_handler = long_press_handler;
    button->double_click_handler = double_click_handler;
    button->user_data = user_data;
    button->filtered_pressed = false;
    button->pressed = false;
    button->released_time_stamp = estc_monotonic_time_get();
    button->pressed_time_stamp = button->released_time_stamp;
}



void estc_button_process_press(ESTCButton * button)
{
    NRF_LOG_INFO("Click");
    ESTCTimeStamp current_time = estc_monotonic_time_get();
    button->pressed_time_stamp = current_time;        
    
    button->pressed = true;
    if (!button->filtered_pressed)
    {
       NRF_LOG_INFO("Filtered Click");
       button->filtered_pressed_time_stamp = current_time;
       button->filtered_pressed = true;        
    }


}


void estc_button_process_release(ESTCButton * button)
{
    ESTCTimeStamp current_time = estc_monotonic_time_get();
    button->released_time_stamp = current_time;
    button->pressed = false;

}




void estc_button_process_update(ESTCButton * button)
{
    ESTCTimeStamp current_time = estc_monotonic_time_get();
    uint32_t released_time = estc_monotonic_time_diff(button->released_time_stamp, current_time);
    uint32_t filtered_pressed_time = estc_monotonic_time_diff(button->filtered_pressed_time_stamp, current_time);

    if (!button->pressed && released_time >= ESTC_BUTTON_BOUNCE_GUARD_TIMEOUT)
    {
        //antibouncing..
        button->filtered_pressed = false;
    }


    if (button->double_click == ESTC_BUTTON_RELEASED_STATE )
    {
        if (button->filtered_pressed)
        {
            button->double_click = ESTC_BUTTON_FIRST_CLICK;
      
        }
    } else if (button->double_click == ESTC_BUTTON_FIRST_CLICK) 
    {
        NRF_LOG_INFO("First click");
        if (button->filtered_pressed)
        {
            if (filtered_pressed_time >= ESTC_BUTTON_PRESS_TIMEOUT)
            {
                NRF_LOG_INFO("Switch to long press state");
                button->double_click =  ESTC_BUTTON_LONG_PRESS;
                if (button->long_press_handler)
                {
                    button->long_press_handler(button->user_data);
                }

            } 
        } else 
           button->double_click = ESTC_BUTTON_WAITING_FOR_SECOND_CLICK;

    } else if (button->double_click == ESTC_BUTTON_WAITING_FOR_SECOND_CLICK)
    {
        NRF_LOG_INFO("Waiting for second click");
        if (button->filtered_pressed)
        {
            if (button->double_click_handler)
                button->double_click_handler(button->user_data);
        } else if (released_time >= ESTC_BUTTON_DOUBLECLICK_TIMEOUT) {
            NRF_LOG_INFO("Switch to released state");
            button->double_click = ESTC_BUTTON_RELEASED_STATE;
        }

    } else if (button->double_click == ESTC_BUTTON_LONG_PRESS)
    {
        if (button->filtered_pressed)
        {
                if (button->long_press_handler)
                {
                    button->long_press_handler(button->user_data);
                }
        } else {
            NRF_LOG_INFO("Switch to released state");
            button->double_click = ESTC_BUTTON_RELEASED_STATE;
        }
    }
    
}