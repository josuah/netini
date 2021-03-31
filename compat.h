#ifndef COMPAT_H
#define COMPAT_H

#include <stddef.h>

/** src/compat/?*.c **/
int strchomp(char *s);
size_t strip(char *s);
size_t strlcpy(char *buf, char const *str, size_t sz);


#endif
