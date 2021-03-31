#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdint.h>

#define die(...) log_die(__VA_ARGS__, 0)
#define warn(...) log_warn(__VA_ARGS__, 0)
#define info(...) log_info(__VA_ARGS__, 0)
#define debug(...) log_debug(__VA_ARGS__, 0)
#define fmt(num) log_num((char[50]){0}, num)

/** src/log.c **/
char * log_num(char *s, uintmax_t i);
void log_vprintf(char const *fmt, va_list va);
void log_die(char const *fmt, ...);
void log_warn(char const *fmt, ...);
void log_info(char const *fmt, ...);
void log_debug(char const *fmt, ...);

#endif
