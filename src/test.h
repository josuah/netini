#ifndef TEST_H
#define TEST_H

#include <stdio.h>

#define TEST_BEGIN	int main(void) {
#define TEST_END	test_summary(); return (0); }
#define test(ok) test_(__LINE__, ok)

static size_t test_count = 0;
static size_t test_err = 0;

static inline void
test_init(void)
{
	fputs("Starting the tests...", stdout);
	fflush(stdout);
}

static inline void
test_lib(char const *s)
{
	fprintf(stdout, "\n\n%s:", s);
	fflush(stdout);
}

static inline void
test_fn(char const *s)
{
	fprintf(stdout, "\n - %-30s", s);
	fflush(stdout);
}

static inline void
test_(long long lnum, int ok)
{
	test_err += !ok;
	test_count++;
	fprintf(stdout, ok ? " .." : " !%lli", lnum);
	fflush(stdout);
}

static inline void
test_summary(void)
{
	fprintf(stdout, "\n\n => %zi/%zi tests passed\n\n", test_count - test_err, test_count);
	fflush(stdout);
}

#endif
