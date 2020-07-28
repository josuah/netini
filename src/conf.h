#ifndef CONF_H
#define CONF_H

#include <stdio.h>

#include "arr.h"
#include "map.h"
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
};

struct conf {
	int init;
	struct mem_pool *pool;
	struct conf_section *current;
	struct arr sections; /* struct conf_section */
};

struct conf_section {
	int init;
	char name[128];
	struct map variables; /* char * */
};

/** src/conf.c **/
char const * conf_strerror(int i);
int conf_parse_variable(struct conf *conf, char *line);
int conf_init_section(struct conf_section *section, char *name, struct mem_pool *pool);
int conf_parse_section(struct conf *conf, char *line);
int conf_parse_line(struct conf *conf, char *line);
int conf_init(struct conf *conf, struct mem_pool *pool);
int conf_getline(char **line, size_t *sz, FILE *fp, size_t *ln);
int conf_parse_stream(struct conf *conf, FILE *fp, size_t *ln, struct mem_pool *pool);
int conf_parse_file(struct conf *conf, char const *path, size_t *ln, struct mem_pool *pool);
struct conf_section * conf_next_section(struct conf *conf, size_t *i, char const *name);
char const * conf_get_section_variable(struct conf_section *section, char *key);
char const * conf_get_variable(struct conf *conf, char *s_name, char *key);
void conf_dump(struct conf *conf, FILE *fp);

#endif
