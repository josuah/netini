#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#include "mem.h"

struct array {
	struct mem_pool *pool;
	int init;
	size_t sz;
	void *mem;
};

/** src/array.c **/
size_t array_length(struct array *arrayay);
void * array_i(struct array *arrayay, size_t pos);
int array_insert(struct array *arrayay, size_t pos, void *value);
int array_append(struct array *arrayay, void *value);
int array_delete(struct array *arrayay, size_t pos);
int array_init(struct array *arrayay, size_t sz, struct mem_pool *pool);

#endif
