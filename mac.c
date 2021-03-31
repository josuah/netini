#include "mac.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>

char const *
mac_parse_addr(char const *s, uint8_t mac[6])
{
	size_t n = 6;

	while (isxdigit(s[0]) && isxdigit(s[1])) {
		uint8_t c0 = tolower(*s++);
		uint8_t c1 = tolower(*s++);

		c0 = (c0 <= '9') ? c0 - '0' : c0 - 'a';
		c1 = (c1 <= '9') ? c1 - '0' : c1 - 'a';

		*mac++ = (c0 << 4) | c1;

		if (--n == 0)
			break;

		if (*s != '-' && *s != ':')
			return NULL;
		s++;
	}
	return s;
}
