#ifndef IP_H
#define IP_H

#include <stdint.h>

#define IP_FMT_ARPA_LEN (128 + sizeof("ip6.arpa"))
#define IP_FMT_ADDR_LEN (128 + sizeof("ip6.arpa"))

/** src/ip.c **/
char const * ip_parse_addr_v4(char const *s, uint8_t ip[4]);
char const * ip_parse_addr_v6(char const *s, uint8_t ip[16]);
char const * ip_parse_addr(char const *s, uint8_t *ip);
char const * ip_parse_mask(char const *s, int version, int *mask);
char const * ip_parse_in_addr_arpa(char const *s, uint8_t *ip, int *prefixlen);
char const * ip_parse_ip6_arpa(char const *s, uint8_t *ip, int *prefixlen);
int ip_version(uint8_t *ip);
int ip_match(uint8_t *ip1, uint8_t *ip2, int prefixlen);
void ip_fmt_arpa_v4(char *s, uint8_t *ip);
void ip_fmt_arpa_v6(char *s, uint8_t *ip);
void ip_fmt_arpa(char *s, uint8_t *ip);
void ip_fmt_addr_v4(char *s, uint8_t *ip);
void ip_fmt_addr_v6(char *s, uint8_t *ip);
void ip_fmt_addr(char *s, uint8_t *ip);

#endif
