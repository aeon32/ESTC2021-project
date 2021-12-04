#ifndef ESTC_STORAGE_H
#define ESTC_STORAGE_H

#include <nrfx_systick.h>

#define ESTC_STORAGE_MAX_DATA_SIZE 255

#pragma pack(push, 1)
//sizeof(StorageRecordHDR) % sizeof(uint32_t) must be == 0, because of alignment
typedef struct _StorageRecordHDR
{
    uint8_t data_type;
    uint8_t data_size;   //size of data(max 255 bytes)
    uint8_t reserved;
    uint8_t crc8;
    uint8_t data[0];
} StorageRecordHDR;

#pragma pack(pop) // disables the effect of #pragma pack from now on

typedef struct
{
    uint8_t * flash_addr;
    uint32_t current_page;
    uint32_t last_record_offset;
    uint32_t freespace_offset;
    StorageRecordHDR * last_record;
} ESTCStorage;

/**
 * Initialization 
**/
void estc_storage_init(ESTCStorage * storage);

/**
 * Save one record
 * Constraints : data_type < 16, data_size < 256
**/
void estc_storage_save_data(ESTCStorage * storage, uint8_t data_type, const void * data, uint8_t data_size);

/**
 *  Get opaque pointer to last record
**/
const StorageRecordHDR * estc_storage_get_last_record(ESTCStorage * storage);

/**
 *  Get saved data
**/
const void * estc_storage_record_data(const StorageRecordHDR * record);

void estc_storage_find_last_record(ESTCStorage * storage);

#endif