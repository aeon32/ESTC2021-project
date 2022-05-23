/**
 *
 * 
*/
#ifndef ESTC_BLE_H__
#define ESTC_BLE_H__

#include <stdint.h>
#include <stdbool.h>

#define ESTC_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
typedef struct estc_ble_struct estc_ble_t;

/**
 * @brief ble initialization with default params
 * returns pointer to filled estc_ble_t structure
 */
estc_ble_t *  estc_ble_init(const char * deviceName, const char * manufacturer);

/**
 * @brief starts advertising
 */
void estc_ble_start(estc_ble_t * estc_ble);

/**
 * @brief getter for connection handle
 * 
 */
uint16_t estc_ble_connection_handle(estc_ble_t * estc_ble);

#endif //ESTC_BLE_H__