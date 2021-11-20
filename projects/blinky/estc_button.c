#include "estc_button.h"
#include <nrf_log.h>
#include <boards.h>




void estc_button_init(ESTCButton * button, ESTCButtonEventHandler double_click_handler, 
                      ESTCButtonEventHandler long_press_handler, void * user_data)
{
    button->double_click = ESTC_BUTTON_NO_DOUBLE_CLICK;   
    button->long_press_handler = long_press_handler;
    button->double_click_handler = double_click_handler;
    button->user_data = user_data;
}



void estc_button_process_click(ESTCButton * button)
{
    NRF_LOG_INFO("Click");
    if (button->double_click == ESTC_BUTTON_NO_DOUBLE_CLICK || button->double_click == ESTC_BUTTON_SECOND_CLICK )
    {
        NRF_LOG_INFO("First Click");
        button->double_click = ESTC_BUTTON_FIRST_CLICK;

        button->first_click_time_stamp = estc_monotonic_time_get();
        

    } else if (button->double_click == ESTC_BUTTON_FIRST_CLICK) 
    {
        NRF_LOG_INFO("Second Click");
        if (!estc_monotonic_time_elapsed_test(button->first_click_time_stamp, ESTC_BUTTON_DOUBLECLICK_TIMEOUT))
        {
            NRF_LOG_INFO("Double click");
            button->double_click = ESTC_BUTTON_SECOND_CLICK;
            if (button->double_click_handler)
            {
                button->double_click_handler(button->user_data);
            }

        } else {
            NRF_LOG_INFO("No Double click");
            button->double_click = ESTC_BUTTON_NO_DOUBLE_CLICK;

        }

    }
}

