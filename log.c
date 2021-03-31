#include "log.h"

/*
 * Log to stderr in the logfmt format through a foolproof API.
 * https://www.brandur.org/logfmt
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
log_errno(int err)
{
	if (err == 0)
		return;
	fputs(" err=\"", stderr);
	fputs(strerror(errno), stderr);
	fputc('"', stderr);
}

char *
log_num(char *buf, uintmax_t i)
{
	char *s = buf;

	for (; i > 0; i /= 10)
		*s++ = '0' + i % 10;
	*s = '\0';
	return buf;
}

static void
log_string(char const *s)
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
log_vprintf(char const *fmt, va_list va)
{
	int first = 1;

	do {
		size_t len = strlen(fmt);

		assert(len > 0);

		if (!first)
			fputc(' ', stderr);
		first = 0;

		fputs(fmt, stderr);
		if (fmt[len - 1] == '=') {
			char const *arg = va_arg(va, char *);
			log_string((arg == NULL) ? "(null)" : arg);
		}
	} while ((fmt = va_arg(va, char *)) != NULL);
}

void
log_die(char const *fmt, ...)
{
	va_list va;

	fputs("error fatal ", stderr);
	va_start(va, fmt);
	log_vprintf(fmt, va);
	log_errno(errno);
	fputc('\n', stderr);
	exit(1);
}

void
log_warn(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	fputs("error ", stderr);
	log_vprintf(fmt, va);
	log_errno(errno);
	fputc('\n', stderr);
	fflush(stderr);
}

void
log_info(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	log_vprintf(fmt, va);
	fputc('\n', stderr);
	fflush(stderr);
}

void
log_debug(char const *fmt, ...)
{
	static int debug_is_on = -1;
	va_list va;

	if (debug_is_on < 0)
		debug_is_on = (getenv("DEBUG") != NULL);
	if (debug_is_on) {
		fputs("debug ", stderr);
		va_start(va, fmt);
		log_vprintf(fmt, va);
		log_errno(errno);
		fputc('\n', stderr);
		fflush(stderr);
	}
}
