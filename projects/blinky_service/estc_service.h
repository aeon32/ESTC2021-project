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

#include <stdint.h>
#include <stdbool.h>

#include "ble.h"
#include "sdk_errors.h"


#define ESTC_BASE_UUID {0x57,0xB7, 0xA8, 0xBF, 0xB0, 0x78, 0x21, 0x43, 0xA6, 0x71, 0x16, 0x88, 0x12, 0x04, 0xA4, 0x64}
#define ESTC_SERVICE_UUID  0x1204 

#define ESTC_GATT_BLINKY_HSV_CHAR 0x1205
#define ESTC_GATT_BLINKY_HSV_CHAR_LEN 12

#define ESTC_GATT_BLINKY_RGB_CHAR 0x1206
#define ESTC_GATT_BLINKY_RGB_CHAR_LEN 18

typedef struct
{
    uint16_t service_handle;
    uint16_t connection_handle;
    ble_uuid_t service_uuid;
    uint8_t hsv_char_value[ESTC_GATT_BLINKY_HSV_CHAR_LEN];
    ble_gatts_char_handles_t hsv_char_handle;
    
    uint8_t rgb_char_value[ESTC_GATT_BLINKY_RGB_CHAR_LEN];
    ble_gatts_char_handles_t rgb_char_handle;


} ble_estc_service_t;


/**
 *  Adds service to ble stack
**/
ret_code_t estc_ble_service_init(ble_estc_service_t *service);

/**
 *  Adds blinky HSV characteristics to service
**/
ret_code_t estc_ble_add_hsv_characteristics(ble_estc_service_t *service);

/**
 *  Adds characteristic
**/
ret_code_t estc_ble_add_characteristic(ble_estc_service_t *service, uint16_t char_id,
                                       const char * description,
                                       uint8_t * char_data, uint16_t char_data_size,
                                       bool read_only, ble_gatts_char_handles_t * out_char_handle);

#endif /* ESTC_SERVICE_H__ */