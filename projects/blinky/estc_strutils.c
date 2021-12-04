#include "estc_strutils.h"
#include <stddef.h>
#include <string.h>

char * estc_strtok_r(char * s, const char * delim, char ** context)
{
	if (!s && !(s = *context))
		return NULL;
	s += strspn(s, delim);
	if (!*s)
		return *context = 0;
	*context = s + strcspn(s, delim);
	if (**context)
		*(*context)++ = 0;
	else
		*context = 0;
	return s;
}
