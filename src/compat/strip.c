#include "compat.h"

#include <string.h>

size_t
strip(char *s)
{
	char *x = s + strlen(s);
	size_t n;

	for (n = 0; --x >= s && (*x == '\t' || *x == ' '); n++)
		*x = '\0';
	return n;
}
