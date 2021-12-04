#ifndef ESTC_UART_TERM_H
#define ESTC_UART_TERM_H

#include <stddef.h>

typedef struct ESTCUARTTerm_
{

} ESTCUARTTerm;

/**
 *  Struct ctor without usbd initialization (it must have been done before)
**/
void estc_uart_term_init(ESTCUARTTerm * estc_uart_term);


/**
 *  Struct ctor with usbd initialization 
**/
void estc_uart_term_init_w_usbd(ESTCUARTTerm * estc_uart_term);


/**
 * Process events
**/
void estc_uart_term_process_events(ESTCUARTTerm * estc_uart_term);

/**
 * Write data to port output buffer. If transmit buffer overflows, data will be lost.
**/
void estc_uart_write(ESTCUARTTerm * estc_uart_term, const char * data, size_t data_size);

#endif