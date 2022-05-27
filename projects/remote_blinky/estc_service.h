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

#ifndef ESTC_SERVICE_H__
#define ESTC_SERVICE_H__

#include "estc_ble.h"
#include "sdk_errors.h"

#include <stdint.h>
#include <stdbool.h>
#include <ble.h>


typedef struct
{
    estc_ble_t * estc_ble;
    uint16_t service_handle;
    ble_uuid_t service_uuid;

} estc_ble_service_t;

/**
 * @brief Initialize service instance, adds it to the stack
 * 
**/
ret_code_t estc_ble_service_init(estc_ble_service_t *service, estc_ble_t * estc_ble, ble_uuid128_t * base_uuid128, uint16_t service_uuid );

/**
 * ESTC service characteristics traits
 * 
 */
enum ESTC_CHAR_FLAGS
{
    ESTC_CHAR_READ = 1,
    ESTC_CHAR_WRITE = 2,
    ESTC_CHAR_NOTIFY = 4,
    ESTC_CHAR_INDICATE = 8,
    ESTC_CHAR_TEXT_FORMAT = 16
};

/**
 *  Adds characteristic to service
**/
ret_code_t estc_ble_add_characteristic(estc_ble_service_t *service, uint16_t char_id,
                                       const char * description,
                                       uint8_t * char_data, uint16_t char_data_size,
                                       uint32_t flags, ble_gatts_char_handles_t * out_char_handle);

/**
 * Sends characteristics notification
 */

ret_code_t estc_char_notify(uint16_t connection_handle, ble_gatts_char_handles_t * char_handle,
                            uint8_t * data, uint16_t data_len );    

/**
 * Sends characteristics indicate
 */

ret_code_t estc_char_indicate(uint16_t connection_handle, ble_gatts_char_handles_t * char_handle,
                            uint8_t * data, uint16_t data_len );                                                                  

#endif /* ESTC_SERVICE_H__ */