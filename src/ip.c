#include "ip.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char const *
ip_parse_addr_v4(char const *s, uint8_t *ip)
{
	unsigned long ul;

	if (!isdigit(*s) || (ul = strtoul(s, (char **)&s, 10)) > 0xff)
		return (NULL);
	ip[0] = ul;

	for (int i = 1; i < 4; i++) {
		if (*s++ != '.')
			return (NULL);

		if (!isdigit(*s) || (ul = strtoul(s, (char **)&s, 10)) > 0xff)
			return (NULL);
		ip[i] = ul;
	}

	return (s);
}

char const *
ip_parse_addr_v6(char const *s, uint8_t *ip)
{
	char const *cp;
	unsigned long ul;
	int i = 0, zpos = 0, zfound = 0;

	if (*s == ':') {
		s++;
		ip[i++] = 0;
		ip[i++] = 0;
	}

	while (i < 16) {
		/* "::" zero compression delimiter */
		if (*s == ':') {
			s++;
			zpos = i;
			zfound = 1;
		}

		/* embeddd IPv4 */
		if ((cp = ip_parse_addr_v4(s, ip + i)) != NULL) {
			i += 4;
			s = cp;
			break;
		}

		/* regular IPv6 */
		if (!isxdigit(*s) || (ul = strtoul(s, (char**)&s, 16)) > 0xffff)
			break;
		ip[i++] = htons(ul) & 0xff;
		ip[i++] = htons(ul) >> 8;

		if (*s++ != ':')
			break;
	}

	if ((zfound && i == 16) || (!zfound && i != 16))
		return (NULL);

	/* shift everything after "::" to the end */
	memmove(ip + 16 - (i - zpos), ip + zpos, i - zpos);
	memset(ip + zpos, 0, 16 - i);

	return (s);
}

char const *
ip_parse_addr(char const *s, uint8_t *ip)
{
	char const *cp;

	if ((cp = ip_parse_addr_v6(s, ip)) != NULL)
		return cp;
	if ((cp = ip_parse_addr_v4(s, ip + 12)) != NULL) {
		memset(ip, 0x00, 10);
		memset(ip + 10, 0xff, 2);
		return cp;
	}
	return NULL;
}

char const *
ip_parse_mask(char const *s, int version, int *mask)
{
	unsigned long ul;

	if (*s++ != '/')
		return (NULL);

	if (!isdigit(*s) || (ul = strtoul(s, (char**)&s, 10)) > 128)
		return (NULL);
	*mask = ul;

	if (version == 4)
		if ((*mask += 96) > 128)
			return (NULL);
	return (s);
}

/*
 * Store a plain text form into <ip> and return the length (in bytes).
 */
char const *
ip_parse_in_addr_arpa(char const *s, uint8_t *ip, int *prefixlen)
{
	uint8_t stack[16];
	unsigned long ul;
	int dots;

	memset(ip, 0, 16);

	/* fill the stack of numbers into stack[] */
	for (dots = 0; dots < 3; dots++) {
		if (!isdigit(*s) || (ul = strtoul(s, (char **)&s, 10)) > 255)
			break;
		stack[dots] = ul;

		if (*s++ != '.')
			return (NULL);
	}
	if (strcmp(s, "in-addr.arpa") != 0)
		return (NULL);

	*prefixlen = dots;

	/* empty the stack of number of stack[] onto (char*)ip */
	while (dots-- > 0)
		*ip++ = stack[dots];

	return (s);
}

char const *
ip_parse_ip6_arpa(char const *s, uint8_t *ip, int *prefixlen)
{
	uint8_t stack[32];
	unsigned long ul;
	int dots = 0;

	memset(ip, 0, 16);

	/* fill the stack of bytes onto ip6[] */
	for (int dots = 0; dots < 32; dots += 2) {
		if (!isxdigit(*s) || (ul = strtoul(s, (char **)&s, 16)) > 0xf)
			break;

		if (*s++ != '.')
			return (NULL);
	}
	if (strcmp(s, "ip6.arpa") != 0)
		return (NULL);

	*prefixlen = dots / 2;

	/* empty the stack onto hex as a hexadecimal string */
	while ((dots -= 2) > 0)
		*ip++ = (stack[dots / 2]) + (stack[dots / 2 + 1] >> 4);

	return (s);
}

int
ip_version(uint8_t *ip)
{
	uint8_t prefix[] = { 0,0,0,0, 0,0,0,0, 0,0,0xff,0xff, 0,0,0,0 };

	return (memcmp(ip, prefix, 12) == 0 ? 4 : 6);
}

/*
 * Match an IP address against a network address for up to prefixlen bytes.
 */
int
ip_match(uint8_t *ip1, uint8_t *ip2, int prefixlen)
{
	uint8_t mask;
	int n = prefixlen / 8;

	if (memcmp(ip1, ip2, n) != 0)
		return (0);
	mask = 0xff ^ (0xff >> prefixlen % 8);
	return ((ip1[n - 1] & mask) == (ip2[n - 1] & mask));
}

void
ip_fmt_arpa_v4(char *s, uint8_t *ip)
{
	for (int i = 3; i >= 0; i--)
		s += sprintf(s, "%d.", ip[i]);
	s += sprintf(s, "in-addr.arpa");
}

void
ip_fmt_arpa_v6(char *s, uint8_t *ip)
{
	int part;

	for (int bits = 128 - 4; bits >= 0; bits -= 4) {
		part = ip[bits/8] >> (4 - bits % 8) & 0xf;
		s += sprintf(s, "%01x.", part);
	}
	s += sprintf(s, "ip6.arpa");
}

void
ip_fmt_arpa(char *s, uint8_t *ip)
{
	switch (ip_version(ip)) {
	case 4:
		ip_fmt_arpa_v4(s, ip);
		break;
	case 6:
		ip_fmt_arpa_v6(s, ip);
		break;
	}
}

void
ip_fmt_addr_v4(char *s, uint8_t *ip)
{
	for (int i = 12, first = 1; i < 16; i++, first = 0)
		s += sprintf(s, first ? "%d" : ".%d", ip[i]);
}

void
ip_fmt_addr_v6(char *s, uint8_t *ip)
{
	uint16_t u16;

	for (int i = 12, first = 1; i < 16; i++, first = 0) {
		u16 = (ip[i + 1]) + (ip[i] << 16);
		s += sprintf(s, first ? "%x" : ":%d", u16);
	}
}

void
ip_fmt_addr(char *s, uint8_t *ip)
{
	switch (ip_version(ip)) {
	case 4:
		ip_fmt_addr_v4(s, ip);
		break;
	case 6:
		ip_fmt_addr_v6(s, ip);
		break;
	}
}
