#include "estc_storage.h"
#include <nrfx_nvmc.h>
#include <nrf_log.h>

#include <nrf_fstorage_sd.h>


#define ESTC_FLASH_START_ADDR 0x3E000
#define ESTC_PAGE_SIZE 0x1000

static const uint8_t CRC_START_VALUE = 0xFE;

static const uint8_t crc8x_table[] = 
{
    0x00, 0x31, 0x62, 0x53, 0xc4, 0xf5, 0xa6, 0x97, 0xb9, 0x88, 0xdb, 0xea, 0x7d,
    0x4c, 0x1f, 0x2e, 0x43, 0x72, 0x21, 0x10, 0x87, 0xb6, 0xe5, 0xd4, 0xfa, 0xcb,
    0x98, 0xa9, 0x3e, 0x0f, 0x5c, 0x6d, 0x86, 0xb7, 0xe4, 0xd5, 0x42, 0x73, 0x20,
    0x11, 0x3f, 0x0e, 0x5d, 0x6c, 0xfb, 0xca, 0x99, 0xa8, 0xc5, 0xf4, 0xa7, 0x96,
    0x01, 0x30, 0x63, 0x52, 0x7c, 0x4d, 0x1e, 0x2f, 0xb8, 0x89, 0xda, 0xeb, 0x3d,
    0x0c, 0x5f, 0x6e, 0xf9, 0xc8, 0x9b, 0xaa, 0x84, 0xb5, 0xe6, 0xd7, 0x40, 0x71,
    0x22, 0x13, 0x7e, 0x4f, 0x1c, 0x2d, 0xba, 0x8b, 0xd8, 0xe9, 0xc7, 0xf6, 0xa5,
    0x94, 0x03, 0x32, 0x61, 0x50, 0xbb, 0x8a, 0xd9, 0xe8, 0x7f, 0x4e, 0x1d, 0x2c,
    0x02, 0x33, 0x60, 0x51, 0xc6, 0xf7, 0xa4, 0x95, 0xf8, 0xc9, 0x9a, 0xab, 0x3c,
    0x0d, 0x5e, 0x6f, 0x41, 0x70, 0x23, 0x12, 0x85, 0xb4, 0xe7, 0xd6, 0x7a, 0x4b,
    0x18, 0x29, 0xbe, 0x8f, 0xdc, 0xed, 0xc3, 0xf2, 0xa1, 0x90, 0x07, 0x36, 0x65,
    0x54, 0x39, 0x08, 0x5b, 0x6a, 0xfd, 0xcc, 0x9f, 0xae, 0x80, 0xb1, 0xe2, 0xd3,
    0x44, 0x75, 0x26, 0x17, 0xfc, 0xcd, 0x9e, 0xaf, 0x38, 0x09, 0x5a, 0x6b, 0x45,
    0x74, 0x27, 0x16, 0x81, 0xb0, 0xe3, 0xd2, 0xbf, 0x8e, 0xdd, 0xec, 0x7b, 0x4a,
    0x19, 0x28, 0x06, 0x37, 0x64, 0x55, 0xc2, 0xf3, 0xa0, 0x91, 0x47, 0x76, 0x25,
    0x14, 0x83, 0xb2, 0xe1, 0xd0, 0xfe, 0xcf, 0x9c, 0xad, 0x3a, 0x0b, 0x58, 0x69,
    0x04, 0x35, 0x66, 0x57, 0xc0, 0xf1, 0xa2, 0x93, 0xbd, 0x8c, 0xdf, 0xee, 0x79,
    0x48, 0x1b, 0x2a, 0xc1, 0xf0, 0xa3, 0x92, 0x05, 0x34, 0x67, 0x56, 0x78, 0x49,
    0x1a, 0x2b, 0xbc, 0x8d, 0xde, 0xef, 0x82, 0xb3, 0xe0, 0xd1, 0x46, 0x77, 0x24,
    0x15, 0x3b, 0x0a, 0x59, 0x68, 0xff, 0xce, 0x9d, 0xac
};

uint8_t static crc8x_fast(uint8_t crc, void const *mem, size_t len) 
{
    uint8_t const *data = mem;
    if (data == NULL)
        return 0xff;
    crc &= 0xff;
    while (len--)
        crc = crc8x_table[crc ^ *data++];
    return crc;
}
static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{

    .evt_handler = fstorage_evt_handler,
    .start_addr = ESTC_FLASH_START_ADDR,
    .end_addr   = ESTC_FLASH_START_ADDR + ESTC_PAGE_SIZE
};

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{

    ESTCStorage * storage = (ESTCStorage * ) p_evt->p_param;
    storage->busy_flag = false;

    if (p_evt->result != NRF_SUCCESS)
    {
        NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }
    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        default:
            break;
    }
}

static void estc_storage_wait_operation_completion(ESTCStorage * storage)
{
    storage->busy_flag = true;
    while(storage->busy_flag)
    {
        sd_app_evt_wait();

    }
}

/**
 * Get the nearest value what is not less than data_size and multiplies by 4 (uint32 boundaries)
**/ 
static uint32_t align_size_to_uint32(uint32_t data_size)
{
    return (data_size % sizeof(uint32_t) ) == 0 ? data_size : (data_size / sizeof(uint32_t)) * sizeof(uint32_t);
}

/**
 * Test for data record.
 * Returns data header if correct data on given offset have been finded.
**/
static uint8_t * estc_storage_test_record(ESTCStorage * storage, uint32_t page, uint32_t offset, StorageRecord * out_record)
{
    NRF_LOG_INFO("Test record offset %x enter", offset);
    StorageRecord aux_record;
    StorageRecord * record_candidate = &aux_record;    
    ret_code_t err_code;
    uint8_t * record_candidate_address = ( (uint8_t *)storage->flash_addr + page * ESTC_PAGE_SIZE + offset);

    //read header
    err_code = nrf_fstorage_read(storage->fstorage, (uint32_t) record_candidate_address,
                               record_candidate, sizeof(record_candidate->hdr));
    
    //APP_ERROR_CHECK(err_code);

    uint32_t data_size_in_bytes = record_candidate->hdr.data_size;
    //datasize in words
    uint32_t aligned_data_size = align_size_to_uint32(data_size_in_bytes);

     NRF_LOG_INFO("Data size is  %d enter code is %d ", data_size_in_bytes, err_code );
    
    if (data_size_in_bytes > ESTC_STORAGE_MAX_DATA_SIZE || (offset + sizeof(StorageRecordHDR) + aligned_data_size) > ESTC_PAGE_SIZE  )
    {
        //data is not correct, record out of the page
        return NULL;
    }

    NRF_LOG_INFO("Data size  %d enter", data_size_in_bytes);
    //read data
    if (data_size_in_bytes > 0)
    {
        err_code = nrf_fstorage_read(storage->fstorage, (uint32_t) record_candidate_address + sizeof(record_candidate->hdr),
                               record_candidate->data, data_size_in_bytes);
        //APP_ERROR_CHECK(err_code);
        NRF_LOG_INFO("Test reading2  %d enter", err_code);
    };

    uint8_t crc = CRC_START_VALUE;    
    crc = crc8x_fast(crc, record_candidate, 3); //crc of header, except of crc itself
    crc = crc8x_fast(crc, (uint8_t *) record_candidate->data, data_size_in_bytes );

    if (crc == record_candidate->hdr.crc8)
    {
        out_record->hdr = record_candidate->hdr;
        memcpy(out_record->data, record_candidate->data, data_size_in_bytes);
        return record_candidate_address;
    } else
    {
        return NULL;
    }

    
}

void estc_storage_find_last_record(ESTCStorage * storage)
{
    uint32_t current_page = 0;
    uint32_t last_record_offset = 0;
    uint8_t * hdr_address;

    while ( (hdr_address = estc_storage_test_record(storage, current_page, last_record_offset, &storage->last_record)) != NULL )
    {
        storage->last_record_offset = last_record_offset;
        storage->last_record_address = hdr_address;

        last_record_offset += sizeof(StorageRecordHDR) + align_size_to_uint32(storage->last_record.hdr.data_size);
    }
    storage->freespace_offset = last_record_offset;
    NRF_LOG_INFO("Record found at %u offset %u", (uint32_t) storage->last_record_address, storage->last_record_offset);  
}

void estc_storage_init(ESTCStorage * storage)
{
    ret_code_t err_code;
    err_code = nrf_fstorage_init(&fstorage, &nrf_fstorage_sd, NULL);
    APP_ERROR_CHECK(err_code);

    storage->flash_addr = (uint8_t *) (fstorage.start_addr );
    storage->current_page = 0;
    storage->last_record_offset = 0;
    storage->last_record_address = NULL;
    storage->freespace_offset = 0;
    storage->fstorage = &fstorage;
    storage->busy_flag = false;
   
    estc_storage_find_last_record(storage);
    //storage is empty - we cannot suppose if storage has been initialized, so erased it.
    if (storage->last_record_address == NULL)
    {
        
        err_code = nrf_fstorage_erase(&fstorage,
                              (uint32_t)storage->flash_addr + storage->current_page * ESTC_PAGE_SIZE ,
                              1,
                              storage);
        APP_ERROR_CHECK(err_code);
        if(err_code == NRF_SUCCESS )
            estc_storage_wait_operation_completion(storage);
        
    }    
  
}

void estc_storage_save_data(ESTCStorage * storage, uint8_t data_type, const void * data, uint8_t data_size)
{

   uint32_t aligned_size = align_size_to_uint32(sizeof(StorageRecordHDR) + data_size);
   if (storage->freespace_offset + aligned_size > ESTC_PAGE_SIZE)
   {
        storage->freespace_offset = 0;
        ret_code_t err_code = nrf_fstorage_erase(storage->fstorage,
                              (uint32_t)storage->flash_addr + storage->current_page * ESTC_PAGE_SIZE ,
                              1,
                              storage);
        if(err_code == NRF_SUCCESS )
            estc_storage_wait_operation_completion(storage);                              
        APP_ERROR_CHECK(err_code);
   }


   //const uint32_t BUFF_SIZE_ALIGNED =  ((sizeof(StorageRecordHDR) + ESTC_STORAGE_MAX_DATA_SIZE) / sizeof(uint32_t) + 1 ) * sizeof(uint32_t);
   ///uint8_t buff[BUFF_SIZE_ALIGNED];
   
   StorageRecord * record_to_save = &storage->last_record;
   record_to_save->hdr.data_type = data_type;
   record_to_save->hdr.data_size = data_size;

   memcpy(storage->last_record.data, data, data_size);

   uint8_t crc = CRC_START_VALUE;    
   crc = crc8x_fast(crc, &record_to_save->hdr, 3); //crc of header, except of crc field itself
   crc = crc8x_fast(crc, record_to_save->data, data_size); //crc of header, except of crc itself
   storage->last_record.hdr.crc8 = crc;

   uint8_t * last_record_addr = storage->flash_addr + storage->current_page * ESTC_PAGE_SIZE + storage->freespace_offset;
  
   nrf_fstorage_write(storage->fstorage,
                      (uint32_t) last_record_addr,
                      record_to_save,
                      aligned_size,
                      NULL);
                  

   storage->last_record_offset = storage->freespace_offset;
   storage->last_record_address = last_record_addr;
   storage->freespace_offset += aligned_size;

   NRF_LOG_INFO("Sizeof storage %u", sizeof(StorageRecordHDR)); 
   NRF_LOG_INFO("Something written at %x offset %u alignedsize %u freespace_offset %u", (uint32_t) storage->last_record_address, storage->last_record_offset, aligned_size, storage->freespace_offset);    
}

const StorageRecordHDR * estc_storage_get_last_record(ESTCStorage * storage)
{
    return storage->last_record_address ? &storage->last_record.hdr : NULL;
}

const void * estc_storage_record_data(const StorageRecordHDR * record)
{
   return  (uint8_t *) record + sizeof(StorageRecordHDR);
}
