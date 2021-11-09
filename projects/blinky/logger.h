#ifndef LOGGER_H
#define LOGGER_H

#include <nrf_log.h>
#include <nrf_log_ctrl.h>
#include <app_usbd.h>


#ifdef __cplusplus
 extern "C" {
#endif

void log_init(void);


#ifdef __cplusplus
}
#endif


#endif