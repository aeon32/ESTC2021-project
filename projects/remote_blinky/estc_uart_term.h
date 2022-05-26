#ifndef ESTC_UART_TERM_H
#define ESTC_UART_TERM_H

#include <stddef.h>

typedef struct ESTCUARTTerm_
{

} ESTCUARTTerm;

/**
 * Handler fires when user enters the command.
 * @param command is null-terminated string; can be modified during call
**/
typedef void (* ESTCUARTTermCommandHandler)(char * command, void * user_data);

/**
 *  Struct ctor without usbd initialization (it must have been done before)
**/
void estc_uart_term_init(ESTCUARTTermCommandHandler command_handler, void * user_data);


/**
 *  Struct ctor with usbd initialization 
**/
void estc_uart_term_init_w_usbd(ESTCUARTTermCommandHandler command_handler, void * user_data);


/**
 * Process events
**/
void estc_uart_term_process_events();

/**
 * Write data to port output buffer. If transmit buffer overflows, data will be lost.
**/
void estc_uart_write(const char * data, size_t data_size);

#endif