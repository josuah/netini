#include "conf.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "array.h"
#include "compat.h"
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
	case CONF_ERR_ENUM_END:
		break;
	}
	assert(!"invalid error code");
	return "unknown error";
}

int
conf_init(struct conf *conf, struct mem_pool *pool)
{
	assert(conf->init == 0);

	if (array_init(&conf->sections, sizeof(struct conf_section), pool) < 0)
		return -CONF_ERR_SYSTEM;

	conf->pool = pool;
	conf->init = 1;
	return 0;
}

static int
conf_parse_variable(struct conf *conf, char *line)
{
	struct conf_variable variable = {0};
	char *buf, *eq, *end;
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

	variable.key = buf;
	end = variable.key + strcspn(variable.key, " \t");
	if (end == variable.key)
		return -CONF_ERR_EMPTY_VARIABLE;
	*end = '\0';

	variable.value = eq + 1;
	variable.value += strspn(variable.value, " \t");

	if (array_append(&conf->current->variables, &variable) < 0)
		return -CONF_ERR_SYSTEM;

	return 0;
}

static int
conf_init_section(struct conf_section *section, char *name,
	struct mem_pool *pool)
{
	size_t sz;

	assert(section->init == 0);

	sz = strlen(name) + 1;
	if (strlcpy(section->name, name, sz) >= sz)
		return -CONF_ERR_SECTION_NAME_TOO_LONG;

	if (array_init(&section->variables, sizeof section->variables, pool) < 0)
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

	if (array_append(&conf->sections, &section) < 0)
		return -CONF_ERR_SYSTEM;

	conf->current = array_i(&conf->sections, array_length(&conf->sections) - 1);
	return 0;
}

static int
conf_parse_line(struct conf *conf, char *line)
{
	if (*line == '[')
		return conf_parse_section(conf, line);
	return conf_parse_variable(conf, line);
}

static int
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
	struct conf_section *section;

	assert(conf->init == 1);
	assert(*i <= array_length(&conf->sections));

	while (*i < array_length(&conf->sections)) {
		section = array_i(&conf->sections, (*i)++);

		if (name == NULL || strcasecmp(section->name, name) == 0)
			return section;
	}
	return NULL;
}

struct conf_variable *
conf_next_variable(struct conf_section *section, size_t *i, char const *key)
{
	struct conf_variable *variable;

	assert(section->init == 1);
	assert(*i <= array_length(&section->variables));

	while (*i < array_length(&section->variables)) {
		variable = array_i(&section->variables, (*i)++);

		if (key == NULL || strcasecmp(variable->key, key) == 0)
			return variable;
	}
	return NULL;
}

char *
conf_next_value(struct conf_section *section, size_t *i, char const *key)
{
	struct conf_variable *variable = conf_next_variable(section, i, key);

	if (variable == NULL)
		return NULL;
	return variable->value;
}

char const *
conf_get_variable(struct conf *conf, char const *s_name, char const *v_name)
{
	struct conf_section *section = NULL;
	size_t i;

	i = 0;
	section = conf_next_section(conf, &i, s_name);
	if (section == NULL)
		return NULL;

	i = 0;
	return conf_next_value(section, &i, v_name);
}

void
conf_dump(struct conf *conf, FILE *fp)
{
	struct conf_section *section;

	for (size_t i = 0; (section = conf_next_section(conf, &i, NULL));) {
		struct array *array = &section->variables;

		fprintf(fp, "%s[%s]\n", (i > 0) ? "\n" : "", section->name);

		for (size_t j = 0; j < array_length(array); j++) {
			struct conf_variable *var = array_i(array, j);

			fprintf(fp, "%s = %s\n", var->key, var->value);
		}
	}
	fprintf(fp, "\n");
}

struct conf_section *
conf_next_matching_section(struct conf *conf, size_t *i1, size_t *i2,
	char const *sect, char const *key, char const *value)
{
	assert(key != NULL);
	assert(value != NULL);

	while (*i1 < array_length(&conf->sections)) {
		struct conf_section *section = array_i(&conf->sections, *i1);
		char const *var;

		while ((var = conf_next_value(section, i2, key)))
			if (strcmp(var, value) == 0)
				return section;
		*i2 = 0;
		if (conf_next_section(conf, i1, sect) == NULL)
			break;
	}
	return NULL;
}
