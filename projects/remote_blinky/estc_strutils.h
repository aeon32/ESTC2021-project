#ifndef ESTC_STRUTILS_H
#define ESTC_STRUTILS_H

/**
 * Implemention of POSIX strtok_r function
**/
char * estc_strtok_r(char * s, const char * delim, char ** context);

#endif