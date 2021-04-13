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
#include "compat.h"

#include <string.h>

size_t
strlcpy(char *buf, char const *str, size_t sz)
{
	size_t len, cpy;

	len = strlen(str);
	cpy = (len > sz) ? (sz) : (len);
	memcpy(buf, str, cpy + 1);
	buf[sz - 1] = '\0';
	return len;
}
