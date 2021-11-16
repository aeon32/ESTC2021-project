#include "estc_button.h"
#include <nrf_log.h>
#include <boards.h>


void estc_button_init(ESTCButton * button)
{
    button->double_click = ESTC_BUTTON_NO_DOUBLE_CLICK;

}


void estc_button_process_click(ESTCButton * button)
{
    NRF_LOG_INFO("Click");
    if (button->double_click == ESTC_BUTTON_NO_DOUBLE_CLICK || button->double_click == ESTC_BUTTON_SECOND_CLICK )
    {
        NRF_LOG_INFO("First Click");
        button->double_click = ESTC_BUTTON_FIRST_CLICK;

        //Is this function reentrant ? I hope so )
        nrfx_systick_get(&button->first_click_time_stamp);

    } else if (button->double_click == ESTC_BUTTON_FIRST_CLICK) 
    {
        NRF_LOG_INFO("Second Click");
        if (!nrfx_systick_test(&button->first_click_time_stamp, ESTC_BUTTON_DOUBLECLICK_TIMEOUT))
        {
            NRF_LOG_INFO("Double click");
            button->double_click = ESTC_BUTTON_SECOND_CLICK;

        } else {
            NRF_LOG_INFO("No Double click");
            button->double_click = ESTC_BUTTON_NO_DOUBLE_CLICK;

        }

    }
}

bool estc_button_have_been_doubleclicked(ESTCButton * button)
{
    bool res = (button->double_click == ESTC_BUTTON_SECOND_CLICK);
    if (res)
    {
        button->double_click = ESTC_BUTTON_NO_DOUBLE_CLICK;

    }
    return res;


};