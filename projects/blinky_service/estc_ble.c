#include "estc_ble.h"

#include <nordic_common.h>
#include <nrf.h>
#include <nrf_sdh.h>
#include <app_error.h>
#include <ble.h>
#include <nrf_log.h>
#include <bsp_btn_ble.h>
#include <nrf_ble_qwr.h>
#include <nrf_sdh_ble.h>
#include <nrf_ble_gatt.h>
#include <ble_advertising.h>

#define ESTC_BLE_APP_ADV_DURATION        18000                                   /**< The advertising duration (180 seconds) in units of 10 milliseconds. */
#define ESTC_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define ESTC_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */


//Generic access parameters
#define ESTC_MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.1 seconds). */
#define ESTC_MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.2 second). */
#define ESTC_SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define ESTC_CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

#define ESTC_MAX_SERVICES_COUNT 1

NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_queud_write);                                                 /**< Context for the Queued Write module.*/

typedef struct estc_ble_struct
{
    bool initialized;
    uint16_t conn_handle;
    ble_uuid_t advert_uuids[ESTC_MAX_SERVICES_COUNT + 1];
    uint16_t services_count;

} estc_ble_t;


estc_ble_t m_estc_ble = {0};

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;
    estc_ble_t * estc_ble = (estc_ble_t *) p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected (conn_handle: %d)", p_ble_evt->evt.gap_evt.conn_handle);
            // LED indication will be changed when advertising starts.
            break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected (conn_handle: %d)", p_ble_evt->evt.gap_evt.conn_handle);

            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);

            estc_ble->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_queud_write, estc_ble->conn_handle);
            APP_ERROR_CHECK(err_code);
            
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request (conn_handle: %d)", p_ble_evt->evt.gap_evt.conn_handle);
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout (conn_handle: %d)", p_ble_evt->evt.gattc_evt.conn_handle);
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout (conn_handle: %d)", p_ble_evt->evt.gatts_evt.conn_handle);
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void estc_gap_params_init(const char * deviceName)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) deviceName,
                                          strlen(deviceName));
    APP_ERROR_CHECK(err_code);

	err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_UNKNOWN);
	APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = ESTC_MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = ESTC_MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = ESTC_SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = ESTC_CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void estc_softdevice_init()
{
    ret_code_t err_code;
    memset(&m_estc_ble, 0, sizeof(estc_ble_t));
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(ESTC_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, ESTC_BLE_OBSERVER_PRIO, ble_evt_handler, &m_estc_ble);    

}


/**@brief Function for initializing the GATT module.
 */
static void estc_gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 */
static void estc_advertising_init()
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    init.advdata.uuids_complete.uuid_cnt = m_estc_ble.services_count + 1;
    init.advdata.uuids_complete.p_uuids  = m_estc_ble.advert_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = ESTC_APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


estc_ble_t * estc_ble_init(const char * deviceName, const char * manufacturer)
{
    NRFX_ASSERT(!m_estc_ble.initialized);
    estc_softdevice_init();
    estc_gap_params_init(deviceName);
    estc_gatt_init();
    estc_advertising_init();
    return &m_estc_ble;

}