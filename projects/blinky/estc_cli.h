#ifndef ESTC_CLI_H
#define ESTC_CLI_H


typedef struct ESTCCLI_ 
{

} ESTCCLI;

/**
 *  Struct ctor
**/
void estc_cli_init(ESTCCLI * estc_cli);

/**
 * Process events
**/
void estc_cli_process_events(ESTCCLI * estc_cli);


#endif