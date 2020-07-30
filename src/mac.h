#ifndef MAC_H
#define MAC_H

#include <stdint.h>

/** src/mac.c **/
char const * mac_parse_addr(char const *s, uint8_t mac[6]);

#endif
