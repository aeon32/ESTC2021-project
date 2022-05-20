/**
 *
 * 
*/
#ifndef ESTC_BLE_H__
#define ESTC_BLE_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t conn_handle;

} estc_ble_t;

/**
 * @brief Single estc_ble_t instance
**/
extern estc_ble_t estc_ble;

/**
 * @brief ble initialization with default params
 * 
 */
void estc_ble_init(const char * deviceName, const char * manufacturer);

#endif //ESTC_BLE_H__