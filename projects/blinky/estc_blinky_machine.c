#include "estc_blinky_machine.h"

#include <boards.h>
#include <nrf_log.h>

/**
 * 
 * 
**/
void estc_blinky_machine_init(ESTCBlinkyMachine* blinky_machine, const uint32_t* sequence_table,
        size_t sequence_table_size, uint32_t blink_period, uint32_t pwm_max_value)
{
    nrfx_systick_get(&blinky_machine->current_pwm_value_start_timestamp);
    blinky_machine->sequence_step = 0;
    blinky_machine->brightness_increasing = true;
    blinky_machine->blink_period = blink_period;
    blinky_machine->pwm_max_value = pwm_max_value;
    blinky_machine->sequence_table = sequence_table;
    blinky_machine->sequence_table_size = sequence_table_size;
    blinky_machine->pwm_value = 0;
}

/**
 * Set new state
 * 
**/
void estc_blinky_machine_next_state(ESTCBlinkyMachine* blinky_machine)
{
    if (nrfx_systick_test(&blinky_machine->current_pwm_value_start_timestamp,
            blinky_machine->blink_period / blinky_machine->pwm_max_value / 2))
    {
        nrfx_systick_get(&blinky_machine->current_pwm_value_start_timestamp);
        if (blinky_machine->brightness_increasing)
        {
            if (blinky_machine->pwm_value == blinky_machine->pwm_max_value)
            {
                //now led is fading..
                blinky_machine->brightness_increasing = false;
            }
            else
            {
                blinky_machine->pwm_value++;

            }
        }
        if (!blinky_machine->brightness_increasing)
        {
            if (blinky_machine->pwm_value == 0)
            {
                //led have done his working cycle, go to the next
                blinky_machine->sequence_step++;
                if (blinky_machine->sequence_step == blinky_machine->sequence_table_size)
                {
                    blinky_machine->sequence_step = 0;
                }
                blinky_machine->brightness_increasing = true;

            }
            else
            {
                blinky_machine->pwm_value--;
            }
        }
    }
}

uint32_t estc_blinky_machine_get_led_pwm(ESTCBlinkyMachine* blinky_machine, uint32_t led_number)
{
    if (blinky_machine->sequence_table[blinky_machine->sequence_step] == led_number)
        return blinky_machine->pwm_value;
    else
        return 0;

}