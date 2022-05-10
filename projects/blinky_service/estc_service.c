/**
 * Copyright 2022 Evgeniy Morozov
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
*/

#include "estc_service.h"

#include "app_error.h"
#include "nrf_log.h"

#include "ble.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"


ret_code_t estc_ble_service_init(ble_estc_service_t *service)
{
    ret_code_t error_code = NRF_SUCCESS;
    memset(service, 0, sizeof(ble_estc_service_t));
     
    
    ble_uuid128_t m_base_uuid128 = {ESTC_BASE_UUID};
    service->service_uuid.uuid = ESTC_SERVICE_UUID;
    error_code = sd_ble_uuid_vs_add(&m_base_uuid128, &service->service_uuid.type);
    APP_ERROR_CHECK(error_code);

    error_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service->service_uuid, &service->service_handle);
    APP_ERROR_CHECK(error_code);

    // TODO: 3. Add service UUIDs to the BLE stack table using `sd_ble_uuid_vs_add`
    // TODO: 4. Add service to the BLE stack using `sd_ble_gatts_service_add`

    NRF_LOG_DEBUG("%s:%d | Service UUID: 0x%04x", __FUNCTION__, __LINE__, service->service_uuid.uuid);
    NRF_LOG_DEBUG("%s:%d | Service UUID type: 0x%02x", __FUNCTION__, __LINE__, service->service_uuid.type);
    NRF_LOG_DEBUG("%s:%d | Service handle: 0x%04x", __FUNCTION__, __LINE__, service->service_handle);

    return NRF_SUCCESS;
}

/**
 *  Adds characteristic
**/
ret_code_t estc_ble_add_characteristic(ble_estc_service_t *service, uint16_t char_id,
                                       const char * description,
                                       uint8_t * char_data, uint16_t char_data_size,
                                       uint32_t flags, ble_gatts_char_handles_t * out_char_handle)
{
    
    ret_code_t error_code = NRF_SUCCESS;

    ble_uuid_t          char_uuid = {0};
    char_uuid.uuid = char_id;
    char_uuid.type = service->service_uuid.type;

    // Configure Characteristic metadata (enable read and write)
    ble_gatts_char_md_t char_md = { 0 };
    char_md.char_props.read   =  flags & ESTC_CHAR_READ ? 1 : 0;
    char_md.char_props.write = flags & ESTC_CHAR_WRITE ? 1 : 0;
    char_md.char_props.notify = flags & ESTC_CHAR_NOTIFY ? 1 : 0;
    char_md.char_props.indicate = flags & ESTC_CHAR_INDICATE ? 1 : 0;
    
    char_md.p_char_user_desc = (uint8_t *) description;
    char_md.char_user_desc_max_size = char_md.char_user_desc_size = strlen(description);

    // Configures attribute metadata. For now we only specify that the attribute will be stored in the softdevice
    ble_gatts_attr_md_t attr_md = { 0 };
    attr_md.vloc = BLE_GATTS_VLOC_STACK;

    // Set read/write security levels to our attribute metadata using `BLE_GAP_CONN_SEC_MODE_SET_OPEN`
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);


    // Configure the characteristic value attribute (set the UUID and metadata)
    ble_gatts_attr_t attr_char_value = { 0 };
    attr_char_value.p_uuid    = &char_uuid;
    attr_char_value.p_attr_md = &attr_md;
    // Set characteristic length in number of bytes in attr_char_value structure
    attr_char_value.init_len  = char_data_size;
    attr_char_value.max_len   = char_data_size;
    attr_char_value.p_value   = char_data;

    // Add new characteristic to the service using 
    error_code = sd_ble_gatts_characteristic_add(service->service_handle, &char_md, &attr_char_value,
                                               out_char_handle);
    APP_ERROR_CHECK(error_code);
    return NRF_SUCCESS;
}

ret_code_t estc_char_notify(uint16_t connection_handle, ble_gatts_char_handles_t * char_handle,
                            uint8_t * data, uint16_t data_len )
{
    ble_gatts_hvx_params_t hvx_params = {0};
    ret_code_t err_code;

    uint16_t len = data_len;

    hvx_params.handle = char_handle->value_handle;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = 0;
    hvx_params.p_len  = &len;
    hvx_params.p_data = data;

    err_code = sd_ble_gatts_hvx(connection_handle, &hvx_params);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
    {
        APP_ERROR_HANDLER(err_code);
    }

    return NRF_SUCCESS;
}   

ret_code_t estc_char_indicate(uint16_t connection_handle ,ble_gatts_char_handles_t * char_handle,
                            uint8_t * data, uint16_t data_len )\
{
ble_gatts_hvx_params_t hvx_params = {0};
    ret_code_t err_code;

    uint16_t len = data_len;

    hvx_params.handle = char_handle->value_handle;
    hvx_params.type   = BLE_GATT_HVX_INDICATION;
    hvx_params.offset = 0;
    hvx_params.p_len  = &len;
    hvx_params.p_data = data;

    err_code = sd_ble_gatts_hvx(connection_handle, &hvx_params);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
    {
        APP_ERROR_HANDLER(err_code);
    }

    return NRF_SUCCESS;    

}                        