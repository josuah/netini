#ifndef IP_H
#define IP_H

#include <stdint.h>

#define IP_FMT_ARPA_LEN (128 + sizeof("ip6.arpa"))
#define IP_FMT_ADDR_LEN (128 + sizeof("ip6.arpa"))

/**/
char const *	ip_parse_addr_v4	(char const *, uint8_t *);
char const *	ip_parse_addr_v6	(char const *, uint8_t *);
char const *	ip_parse_addr		(char const *, uint8_t *);
char const *	ip_parse_mask		(char const *, int, int *);
char const *	ip_parse_in_addr_arpa	(char const *, uint8_t *, int *);
char const *	ip_parse_ip6_arpa	(char const *, uint8_t *, int *);
int		ip_version		(uint8_t *);
int		ip_match		(uint8_t *, uint8_t *, int);
void		ip_fmt_arpa_v4		(char *, uint8_t *);
void		ip_fmt_arpa_v6		(char *, uint8_t *);
void		ip_fmt_arpa		(char *, uint8_t *);
void		ip_fmt_addr_v4		(char *, uint8_t *);
void		ip_fmt_addr_v6		(char *, uint8_t *);
void		ip_fmt_addr		(char *, uint8_t *);

#endif
