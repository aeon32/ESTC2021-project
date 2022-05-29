/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup estc_gatt main.c
 * @{
 * @ingroup estc_templates
 * @brief ESTC-GATT project file.
 *
 * This file contains a template for creating a new BLE application with GATT services. It has
 * the code necessary to advertise, get a connection, restart advertising on disconnect.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "sensorsim.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "estc_service.h"
#include "estc_ble.h"

#define DEVICE_NAME                     "ESTColour"                                /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "NordicSemiconductor"                   /**< Manufacturer. Will be passed to Device Information Service. */


#define ESTC_BASE_UUID {0x57,0xB7, 0xA8, 0xBF, 0xB0, 0x78, 0x21, 0x43, 0xA6, 0x71, 0x16, 0x88, 0x12, 0x04, 0xA4, 0x64}
#define ESTC_SERVICE_UUID  0x1204 


#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */



#define APP_CFG_CHAR_NOTIF_TIMEOUT  1000                                        /**< Notifications period (in milli seconds). */  
#define APP_CFG_CHAR_INDICATE_TIMEOUT  2500                                     /**< Indications period (in milli seconds). */  

APP_TIMER_DEF(m_indication_timer_id);                                           /**< Indication timer. */
APP_TIMER_DEF(m_notif_timer_id);                                                /**< Notification timer. */


#define ESTC_GATT_BLINKY_HSV_CHAR 0x1205
#define ESTC_GATT_BLINKY_HSV_CHAR_LEN 12

#define ESTC_GATT_BLINKY_RGB_CHAR 0x1206
#define ESTC_GATT_BLINKY_RGB_CHAR_LEN 18

#define ESTC_GATT_BLINKY_INDICATE_CHAR 0x1207
#define ESTC_GATT_BLINKY_NOTIFY_CHAR 0x1208


typedef struct 
{
    estc_ble_service_t base; 
    
    //characteristics data
    uint8_t hsv_char_value[ESTC_GATT_BLINKY_HSV_CHAR_LEN];
    ble_gatts_char_handles_t hsv_char_handle;
    
    uint8_t rgb_char_value[ESTC_GATT_BLINKY_RGB_CHAR_LEN];
    ble_gatts_char_handles_t rgb_char_handle;

    uint32_t indicate_char_value;
    ble_gatts_char_handles_t indicate_char_handle;
    
    uint32_t notify_char_value;
    ble_gatts_char_handles_t notify_char_handle;

} blinky_service_t;

blinky_service_t m_blinky_service;

static void notification_interval_timeout_handler(void * p_context)
{
    
    blinky_service_t * blinky_service = (blinky_service_t * ) p_context;
    NRF_LOG_INFO("Notify timeout");
    blinky_service->notify_char_value++;
    
    estc_char_notify(estc_ble_connection_handle(blinky_service->base.estc_ble), &blinky_service->notify_char_handle, 
                    (uint8_t *) &blinky_service->notify_char_value, sizeof(blinky_service->notify_char_value) );
    
}
    

/**
 * Function fo sending indication using timeouts
 *
 */

static void indicate_interval_timeout_handler(void * p_context)
{
    blinky_service_t * blinky_service = (blinky_service_t * ) p_context;
    NRF_LOG_INFO("Indicate timeout");
    blinky_service->indicate_char_value++;

    estc_char_indicate(estc_ble_connection_handle(blinky_service->base.estc_ble), &blinky_service->indicate_char_handle, 
                    (uint8_t *) &blinky_service->indicate_char_value, sizeof(blinky_service->indicate_char_value) );

}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void chars_timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_notif_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                notification_interval_timeout_handler);

    APP_ERROR_CHECK(err_code);                                

    err_code = app_timer_create(&m_indication_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                indicate_interval_timeout_handler);
    APP_ERROR_CHECK(err_code);                                 
}





/**@brief Function for starting timers.
 */
static void chars_timers_start(blinky_service_t * service)
{
    ret_code_t err_code;
    err_code = app_timer_start(m_notif_timer_id, APP_TIMER_TICKS(APP_CFG_CHAR_NOTIF_TIMEOUT), service);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_indication_timer_id, APP_TIMER_TICKS(APP_CFG_CHAR_INDICATE_TIMEOUT), service);
    APP_ERROR_CHECK(err_code);
}


static void chars_timers_stop(void)
{
    ret_code_t err_code;
    err_code = app_timer_stop(m_notif_timer_id);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_stop(m_indication_timer_id);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(void)
{
    ret_code_t err_code;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
	LOG_BACKEND_USB_PROCESS();
}


/**@brief Function for adding characteristics
 */
static void blinky_service_add_chars(blinky_service_t * blinky_service)
{
    ret_code_t         err_code;
    uint32_t deadbeef = DEAD_BEEF;
    memcpy(blinky_service->hsv_char_value, &deadbeef, sizeof(deadbeef) );

    uint32_t rgb_value = 0xAABBCCDD;
    memcpy(blinky_service->rgb_char_value, &rgb_value, sizeof(rgb_value) );


    // Add characteristics, write/read and readonly
    err_code = estc_ble_add_characteristic(&blinky_service->base, ESTC_GATT_BLINKY_HSV_CHAR, 
                                           "HSV colour value", blinky_service->hsv_char_value, 
                                           ESTC_GATT_BLINKY_HSV_CHAR_LEN, ESTC_CHAR_READ | ESTC_CHAR_WRITE, &blinky_service->hsv_char_handle);
    APP_ERROR_CHECK(err_code);

    err_code = estc_ble_add_characteristic(&blinky_service->base, ESTC_GATT_BLINKY_RGB_CHAR, 
                                           "RGB colour value", blinky_service->rgb_char_value, 
                                           ESTC_GATT_BLINKY_RGB_CHAR_LEN, ESTC_CHAR_READ, &blinky_service->rgb_char_handle);
    APP_ERROR_CHECK(err_code);


    // Add characteristics, indicate and notify
    err_code = estc_ble_add_characteristic(&blinky_service->base, ESTC_GATT_BLINKY_INDICATE_CHAR, 
                                           "Indicate char ", (uint8_t *) &blinky_service->indicate_char_value, 
                                           sizeof(blinky_service->indicate_char_value), ESTC_CHAR_READ | ESTC_CHAR_INDICATE, &blinky_service->indicate_char_handle);
    APP_ERROR_CHECK(err_code);

    err_code = estc_ble_add_characteristic(&blinky_service->base, ESTC_GATT_BLINKY_NOTIFY_CHAR, 
                                           "Notify char", (uint8_t *) &blinky_service->notify_char_value,
                                           sizeof(blinky_service->notify_char_value), ESTC_CHAR_READ | ESTC_CHAR_NOTIFY, &blinky_service->notify_char_handle);
    APP_ERROR_CHECK(err_code);

}


static void connection_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    blinky_service_t * blinky_service  = (blinky_service_t *) p_context;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            // LED indication will be changed when advertising starts.
            NRF_LOG_INFO("Stop notifying");
            chars_timers_stop();            
            break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Start notifying");
            chars_timers_start(blinky_service);
            break;


        default:
            // No implementation needed.
            break;
    }
}


static void blinky_service_init(blinky_service_t * blinky_service, estc_ble_t * estc_ble)
{
   
   memset(blinky_service, 0, sizeof(blinky_service_t));
   ble_uuid128_t  base_uuid128 = {ESTC_BASE_UUID};

   ret_code_t err_code = estc_ble_service_init(&blinky_service->base, estc_ble, &base_uuid128, ESTC_SERVICE_UUID );
   APP_ERROR_CHECK(err_code);

   
   blinky_service_add_chars(blinky_service);
   NRF_SDH_BLE_OBSERVER(m_connection_observer_observer, ESTC_BLE_OBSERVER_PRIO, connection_handler, &m_blinky_service);    
}

int main(void)
{
   log_init();
   chars_timers_init();
   buttons_leds_init();
   power_management_init();

   estc_ble_t * estc_ble = estc_ble_init(DEVICE_NAME, MANUFACTURER_NAME);  
   blinky_service_init(&m_blinky_service, estc_ble);
     
   // Start execution.
   NRF_LOG_INFO("ESTC GATT service example started");
   estc_ble_start(estc_ble);

    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
    }

}

/**
 * @}
 */
