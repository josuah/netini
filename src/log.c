#include "log.h"

/*
 * Log to stderr with extra data (errno, program name, ...) added.
 * https://www.brandur.org/logfmt
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *arg0 = NULL;

static void
log_string(char *s)
{
	if (s[strcspn(s, " \t")] == '\0') {
		fputs(s, stderr);
	} else {
		fputc('"', stderr);
		fputs(s, stderr);
		fputc('"', stderr);
	}
}

void
log_vprintf(char const *level, char const *fmt, va_list va)
{
	int old_errno = errno;

	if (arg0 != NULL)
		fprintf(stderr, "prog=%s ", arg0);
	fprintf(stderr, "level=%s ", level);

	for (; *fmt != '\0'; fmt++) {
		if (*fmt == '%') {
			switch (*++fmt) {
			case 'd':
				vfprintf(stderr, "%d", va);
				break;
			case 's':
				log_string(va_arg(va, char *));
				break;
			default:
				assert(!"unknown char after percent");
			}
		} else {
			fputc(*fmt, stderr);
		}
	}
	if (old_errno)
		fprintf(stderr, " err=\"%s\"", strerror(old_errno));
	fputc('\n', stderr);
	fflush(stderr);
}

void
die(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	log_vprintf("error", fmt, va);
	exit(1);
}

void
warn(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	log_vprintf("warn", fmt, va);
}

void
info(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	log_vprintf("info", fmt, va);
}

void
debug(char const *fmt, ...)
{
	static int debug_is_on = -1;
	va_list va;

	if (debug_is_on < 0)
		debug_is_on = (getenv("DEBUG") != NULL);
	if (debug_is_on) {
		va_start(va, fmt);
		log_vprintf("debug", fmt, va);
	}
}
