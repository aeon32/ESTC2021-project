#include "estc_cli.h"

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

static char m_rx_buffer[READ_SIZE];
static char m_tx_buffer[NRF_DRV_USBD_EPSIZE];
static bool m_send_flag = true;

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);
    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
            /*Setup first transfer*/
            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   m_rx_buffer,
                                                   READ_SIZE);
            UNUSED_VARIABLE(ret);
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            break;
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            m_send_flag = true;

            break;
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            ret_code_t ret;
            NRF_LOG_INFO("Bytes waiting: %d", app_usbd_cdc_acm_bytes_stored(p_cdc_acm));
            do
            {
                /*Get amount of data transfered*/
                size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);
                NRF_LOG_INFO("RX: size: %lu char: %c", size, m_rx_buffer[0]);

                /* Fetch data until internal buffer is empty */
                ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                            m_rx_buffer,
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


void estc_cli_init(ESTCCLI * estc_cli)
{
    ret_code_t ret;

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

    if (!nrf_drv_usbd_is_enabled())
    {
        app_usbd_enable();
    }
    if (!nrf_drv_usbd_is_started())
    {
        app_usbd_start();
    }

}

void estc_cli_process_events(ESTCCLI * estc_cli)
{
   
    while (app_usbd_event_queue_process())
    {
        /* Nothing to do */
    }
    
    if(m_send_flag)
    {
        size_t size = sprintf(m_tx_buffer, "Hello USB CDC FA demo: \r\n");
        app_usbd_cdc_acm_write(&m_app_cdc_acm, m_tx_buffer, size);
        m_send_flag = false;
    }
    
}

