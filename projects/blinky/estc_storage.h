#ifndef ESTC_STORAGE_H
#define ESTC_STORAGE_H

#include <nrfx_systick.h>

//maximum possible data len
#define ESTC_STORAGE_MAX_DATA_LEN (0x1F * 4)

typedef struct
{
    void * flash_addr;
    uint32_t current_page;
    uint32_t last_record_offset;
    void * last_record;
    
} ESTCStorage;

/**
 * Initialization 
**/
void estc_storage_init(ESTCStorage * storage);

/**
 * Save one record
**/
void estc_storage_save_data(ESTCStorage * storage);

#endif