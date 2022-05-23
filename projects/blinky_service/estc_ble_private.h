#ifndef ESTC_BLE_PRIVATE_H__
#define ESTC_BLE_PRIVATE_H__

#define ESTC_MAX_SERVICES_COUNT 1

#include <ble.h>
#include <stdint.h>

typedef struct estc_ble_struct
{
    bool initialized;
    uint16_t conn_handle;
    ble_uuid_t advert_uuids[ESTC_MAX_SERVICES_COUNT + 1];
    uint16_t services_count;

} estc_ble_t;


#endif //ESTC_BLE_PRIVATE_H__