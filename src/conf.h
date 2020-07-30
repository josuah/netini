#ifndef CONF_H
#define CONF_H

#include <stdio.h>

#include "arr.h"
#include "mem.h"

enum conf_errno {
	CONF_ERR_SYSTEM = 1,
	CONF_ERR_NUL_BYTE,
	CONF_ERR_SECTION_NAME_TOO_LONG,
	CONF_ERR_MISSING_CLOSING_BRACKET,
	CONF_ERR_MISSING_EQUAL,
	CONF_ERR_VARIABLE_BEFORE_SECTION,
	CONF_ERR_EMPTY_VARIABLE,
	CONF_ERR_EXTRA_AFTER_SECTION,
	CONF_ERR_ENUM_END,
};

struct conf {
	int init;
	struct mem_pool *pool;
	struct conf_section *current;
	struct arr sections; /* struct conf_section */
};

struct conf_section {
	int init;
	char name[64];
	struct arr variables; /* struct conf_variable */
};

struct conf_variable {
	char *key, *value;
	char buf[];
};

/** src/conf.c **/
char const * conf_strerror(int i);
int conf_init(struct conf *conf, struct mem_pool *pool);
int conf_parse_section(struct conf *conf, char *line);
int conf_parse_stream(struct conf *conf, FILE *fp, size_t *ln, struct mem_pool *pool);
int conf_parse_file(struct conf *conf, char const *path, size_t *ln, struct mem_pool *pool);
struct conf_section * conf_next_section(struct conf *conf, size_t *i, char const *name);
struct conf_variable * conf_next_variable(struct conf_section *section, size_t *i, char *key);
char * conf_next_value(struct conf_section *section, size_t *i, char *key);
char const * conf_get_variable(struct conf *conf, char *s_name, char *v_name);
void conf_dump(struct conf *conf, FILE *fp);

#endif
