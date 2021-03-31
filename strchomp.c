#include "compat.h"

#include <string.h>

/*
 * Remove one newline character from the end of the string if any
 * and return 1, or 0 if there is not one.
 */
int
strchomp(char *s)
{
	size_t len;

	len = strlen(s);
	if (len > 0 && s[len - 1] == '\n') {
		s[len - 1] = '\0';
		return 1;
	}
	return 0;
}
