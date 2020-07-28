#include "conf.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "arr.h"
#include "compat.h"
#include "map.h"
#include "mem.h"

/*
 * Parser for config.ini configuration format.
 *
 *	# comment and everything can have leading spaces/tabs
 *
 *	[section]
 *	  case_insensitive = unquoted value #<- included in the value
 *	  there_can_be_spaces_before_and_after = they will be trimmed
 *
 *	[object]
 *	  multiple_sections_with_same_name = allowed
 *
 *	[object]
 *	  # empty sections allowed
 */

char const *
conf_strerror(int i)
{
	enum conf_errno err = (i > 0) ? i : -i;

	switch (err) {
	case CONF_ERR_SYSTEM:
		return "system error";
	case CONF_ERR_NUL_BYTE:
		return "'\\0' byte in configuration";
	case CONF_ERR_SECTION_NAME_TOO_LONG:
		return "section name is too long for this library";
	case CONF_ERR_MISSING_CLOSING_BRACKET:
		return "expecting closing bracket";
	case CONF_ERR_MISSING_EQUAL:
		return "expecting equal sign in variable assignment line";
	case CONF_ERR_VARIABLE_BEFORE_SECTION:
		return "unexpected variable before the first section";
	case CONF_ERR_EMPTY_VARIABLE:
		return "variable name must be at least one character long";
	case CONF_ERR_EXTRA_AFTER_SECTION:
		return "unexpected data found after section name";
	}
	assert(!"invalid error code");
	return "unknown error";
}

int
conf_parse_variable(struct conf *conf, char *line)
{
	char *buf, *eq, *val, *key, *end;
	size_t len = strlen(line);

	assert(conf->init == 1);

	if (conf->current == NULL)
		return -CONF_ERR_VARIABLE_BEFORE_SECTION;

	buf = mem_alloc(conf->pool, len + 1);
	if (buf == NULL)
		return -CONF_ERR_SYSTEM;
	memcpy(buf, line, len + 1);

	eq = strchr(buf, '=');
	if (eq == NULL)
		return -CONF_ERR_MISSING_EQUAL;
	*eq = '\0';

	key = buf;
	end = key + strcspn(key, " \t");
	if (end == key)
		return -CONF_ERR_EMPTY_VARIABLE;
	*end = '\0';

	val = eq + 1;
	val += strspn(val, " \t");

	if (map_set(&conf->current->variables, key, val) < 0)
		return -CONF_ERR_SYSTEM;

	return 0;
}

int
conf_init_section(struct conf_section *section, char *name,
	struct mem_pool *pool)
{
	size_t sz;

	assert(section->init == 0);

	sz = strlen(name) + 1;
	if (strlcpy(section->name, name, sz) >= sz)
		return -CONF_ERR_SECTION_NAME_TOO_LONG;

	if (map_init(&section->variables, pool) < 0)
		return -CONF_ERR_SYSTEM;

	section->init = 1;
	return 0;
}

int
conf_parse_section(struct conf *conf, char *line)
{
	struct conf_section section = {0};
	char *s;
	int err;

	assert(conf->init == 1);
	assert(*line == '[');

	line++;
	for (s = line; strchr("]\0", *s) == NULL; s++)
		*s = tolower(*s);
	if (*s != ']')
		return -CONF_ERR_MISSING_CLOSING_BRACKET;
	if (s[1] != '\0')
		return -CONF_ERR_EXTRA_AFTER_SECTION;
	*s = '\0';

	err = conf_init_section(&section, line, conf->pool) ;
	if (err < 0)
		return err;

	if (arr_append(&conf->sections, &section) < 0)
		return -CONF_ERR_SYSTEM;

	conf->current = arr_i(&conf->sections, arr_length(&conf->sections) - 1);
	return 0;
}

int
conf_parse_line(struct conf *conf, char *line)
{
	if (*line == '[')
		return conf_parse_section(conf, line);
	return conf_parse_variable(conf, line);
}

int
conf_init(struct conf *conf, struct mem_pool *pool)
{
	assert(conf->init == 0);

	if (arr_init(&conf->sections, sizeof(struct conf_section), pool) < 0)
		return -CONF_ERR_SYSTEM;

	conf->pool = pool;
	conf->init = 1;
	return 0;
}

int
conf_getline(char **line, size_t *sz, FILE *fp, size_t *ln)
{
	ssize_t r, i;

top:
	r = getline(line, sz, fp);
	if (ferror(fp))
		return -CONF_ERR_SYSTEM;
	if (r <= 0)
		return 0;

	(*ln)++;

	if (memchr(*line, '\0', r) != NULL)
		return -CONF_ERR_NUL_BYTE;
	r -= strchomp(*line);
	r -= strip(*line);
	assert(r >= 0);

	i = strspn(*line, " \t");
	for (ssize_t o = 0; i < r; o++, i++)
		(*line)[o] = (*line)[i];

	if ((*line)[0] == '#' || (*line)[0] == '\0')
		goto top;

	return 1;
}

int
conf_parse_stream(struct conf *conf, FILE *fp, size_t *ln,
	struct mem_pool *pool)
{
	char *line = NULL;
	size_t sz = 0;
	int err;

	if (conf_init(conf, pool) < 0)
		return -CONF_ERR_SYSTEM;

	*ln = 0;
	while ((err = conf_getline(&line, &sz, fp, ln)) > 0) {
		err = conf_parse_line(conf, line);
		if (err < 0)
			break;
	}
	free(line);
	return err;
}

int
conf_parse_file(struct conf *conf, char const *path, size_t *ln,
	struct mem_pool *pool)
{
	FILE *fp;

	fp = fopen(path, "r");
	if (fp == NULL)
		return -CONF_ERR_SYSTEM;

	return conf_parse_stream(conf, fp, ln, pool);
}

struct conf_section *
conf_next_section(struct conf *conf, size_t *i, char const *name)
{
	assert(conf->init == 1);
	assert(*i <= arr_length(&conf->sections));

	while (*i < arr_length(&conf->sections)) {
		struct conf_section *section = arr_i(&conf->sections, (*i)++);

		if (name == NULL || strcasecmp(section->name, name) == 0)
			return section;
	}
	return NULL;
}

char const *
conf_get_section_variable(struct conf_section *section, char *key)
{
	assert(section->init == 1);

	return map_get(&section->variables, key);
}

char const *
conf_get_variable(struct conf *conf, char *s_name, char *key)
{
	struct conf_section *section = NULL, *se = NULL;
	size_t i = 0;

	while ((se = conf_next_section(conf, &i, s_name)) != NULL)
		section = se;
	if (section == NULL)
		return NULL;
	return conf_get_section_variable(section, key);
}

void
conf_dump(struct conf *conf, FILE *fp)
{
	struct conf_section *section;

	for (size_t i = 0; (section = conf_next_section(conf, &i, NULL));) {
		struct map *map = &section->variables;

		fprintf(fp, "\n[%s]\n\n", section->name);

		for (size_t j = 0; j < map_length(map); j++) {
			struct map_entry *entry = arr_i(&map->array, j);
			char *key = entry->key, *value = entry->value;

			fprintf(fp, "%s = %s\n", key, value);
		}
	}
	fprintf(fp, "\n");
}
