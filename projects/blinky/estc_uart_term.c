#include "estc_uart_term.h"

#include <nrfx_nvmc.h>
#include <nrf_log.h>

#include <nrf_drv_clock.h>
#include <app_timer.h>
#include <app_error.h>
#include <app_util.h>
#include <app_usbd_core.h>
#include <app_usbd.h>
#include <app_usbd_string_desc.h>
#include <app_usbd_cdc_acm.h>
#include <app_usbd_serial_num.h>


static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN4

#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN3
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT3

#ifndef USBD_POWER_DETECTION
#define USBD_POWER_DETECTION true
#endif

/**
 * @brief CDC_ACM class instance
 * */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);

#define READ_SIZE 1
#define WRITE_BUFFER_SIZE 256
#define ESTC_MAX_COMMAND_SIZE 63

typedef struct ESTCUARTTermInternal_
{
    char rx_buffer[READ_SIZE];
    char tx_buffer[WRITE_BUFFER_SIZE];
    
    size_t tx_buffer_offset;
    //size of last pending write
    size_t tx_len;

    char command[ESTC_MAX_COMMAND_SIZE + 1];
    size_t command_offset;

    ESTCUARTTermCommandHandler command_handler;
    void * user_data;
} ESTCUARTTermInternal;

static ESTCUARTTermInternal term_internal = 
     { .tx_buffer_offset = 0, .tx_len = 0, .command_offset = 0, .command_handler = NULL, .user_data = NULL};


static void estc_uart_term_append_to_command(ESTCUARTTerm * estc_uart_term, const char * data, size_t data_size)
{
    if (term_internal.command_offset < sizeof(term_internal.command))
    {
      
        size_t available_space = ESTC_MAX_COMMAND_SIZE - term_internal.command_offset;
        size_t data_to_write_size = data_size < available_space ? data_size : available_space;

        memcpy(&term_internal.command[term_internal.command_offset], data, data_to_write_size);
        term_internal.command_offset += data_to_write_size;
    }    
}

static void estc_uart_term_command_fired(ESTCUARTTerm * estc_uart_term)
{
    term_internal.command[term_internal.command_offset] = '\0'; 
    term_internal.command_offset = 0;

    if (term_internal.command_handler)
    {
        term_internal.command_handler(term_internal.command, term_internal.user_data);
    }
}

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    //app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);
    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
            /*Setup first transfer*/
            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   term_internal.rx_buffer,
                                                   READ_SIZE);
            UNUSED_VARIABLE(ret);
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            break;
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            //
            if (term_internal.tx_len < term_internal.tx_buffer_offset)
            {   
                //we should move untransmitted data to array beginning
                memmove(&term_internal.tx_buffer[0], &term_internal.tx_buffer[0] + term_internal.tx_len, 
                    term_internal.tx_buffer_offset - term_internal.tx_len);
                term_internal.tx_buffer_offset -= term_internal.tx_len;
            } 
            else
            {
                term_internal.tx_buffer_offset = 0;
            }
            term_internal.tx_len = 0;

            break;
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            ret_code_t ret;
            do
            {
                if (term_internal.rx_buffer[0] == '\r') 
                {
                    estc_uart_write(NULL, "\r\n", 2);
                    estc_uart_term_command_fired(NULL);
                } 
                else
                {
                    estc_uart_write(NULL, &term_internal.rx_buffer[0], 1);
                    estc_uart_term_append_to_command(NULL, &term_internal.rx_buffer[0], 1);
                }
                
                /* Fetch data until internal buffer is empty */
                ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                            term_internal.rx_buffer,
                                            READ_SIZE);
            } while (ret == NRF_SUCCESS);
            break;
        }
        default:
            break;
    }    
}

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SUSPEND:
             break;
        case APP_USBD_EVT_DRV_RESUME:
             break;
        case APP_USBD_EVT_STARTED:
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            NRF_LOG_INFO("USB power detected");
            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            app_usbd_start();
            break;
        default:
            break;
    }
}

static const app_usbd_config_t usbd_config = 
{
    .ev_state_proc = usbd_user_ev_handler
};


void estc_uart_term_init(ESTCUARTTerm * estc_uart_term, ESTCUARTTermCommandHandler command_handler, void * user_data)
{
    ret_code_t ret;
    term_internal.command_handler = command_handler;
    term_internal.user_data = user_data;
    app_usbd_serial_num_generate();
    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret = app_usbd_class_append(class_cdc_acm);
    UNUSED_VARIABLE(ret);
}



void estc_uart_term_init_w_usbd(ESTCUARTTerm * estc_uart_term, ESTCUARTTermCommandHandler command_handler, void * user_data)
{
    ret_code_t ret;
    term_internal.command_handler = command_handler;
    term_internal.user_data = user_data;    
    if (!nrf_drv_clock_init_check())
    {
        ret = nrf_drv_clock_init();
        APP_ERROR_CHECK(ret);
    }
    /* Start up the Low Frequency clocking if it is not running already */
    if (!nrf_drv_clock_lfclk_is_running())
    {
        nrf_drv_clock_lfclk_request(NULL);
        while (!nrf_drv_clock_lfclk_is_running())
        {
            /* Just waiting until the lf clock starts up */
        }
    }
    app_usbd_serial_num_generate();

    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);
    NRF_LOG_INFO("USBD CDC ACM example started.");
    
    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);

    return;

    if (USBD_POWER_DETECTION)
    {
        ret = app_usbd_power_events_enable();
        APP_ERROR_CHECK(ret);
    }
    else
    {
        if (!nrf_drv_usbd_is_enabled())
        {
            app_usbd_enable();
        }
        if (!nrf_drv_usbd_is_started())
        {
            app_usbd_start();
        }
    }
   
}

void estc_uart_term_process_events(ESTCUARTTerm * estc_uart_term)
{
   
    while (app_usbd_event_queue_process())
    {
        /* Nothing to do */
    }
    
    //we have data to write and last write operation finished
    if(term_internal.tx_buffer_offset > 0 && term_internal.tx_len == 0)
    {
        
        size_t data_to_send_size = term_internal.tx_buffer_offset < NRF_DRV_USBD_EPSIZE ? term_internal.tx_buffer_offset : NRF_DRV_USBD_EPSIZE;

        app_usbd_cdc_acm_write(&m_app_cdc_acm, term_internal.tx_buffer, data_to_send_size);

        term_internal.tx_len = data_to_send_size;
    }
}

void estc_uart_write(ESTCUARTTerm * estc_uart_term, const char * data, size_t data_size)
{
    if (term_internal.tx_buffer_offset < sizeof(term_internal.tx_buffer))
    {
        //copy data to the internal buffer ( if space is available)
        size_t available_space = sizeof(term_internal.tx_buffer) - term_internal.tx_buffer_offset;
        size_t data_to_write_size = data_size < available_space ? data_size : available_space;

        memcpy(&term_internal.tx_buffer[term_internal.tx_buffer_offset], data, data_to_write_size);


        term_internal.tx_buffer_offset += data_to_write_size;
    }    
}
