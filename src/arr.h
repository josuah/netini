#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#include "mem.h"

struct arr {
	int init;
	size_t sz;
	void *mem;
};

/** src/arr.c **/
size_t arr_length(struct arr *array);
void * arr_i(struct arr *array, size_t pos);
int arr_insert(struct arr *array, size_t pos, void *value);
int arr_append(struct arr *array, void *value);
int arr_delete(struct arr *array, size_t pos);
int arr_init(struct arr *array, size_t sz, struct mem_pool *pool);

#endif
