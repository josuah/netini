#include "arr.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"

size_t
arr_length(struct arr *array)
{
	assert(array->init == 1);

	return mem_length(array->mem) / array->sz;
}

void *
arr_i(struct arr *array, size_t pos)
{
	assert(array->init == 1);
	assert(pos < arr_length(array));

	return (char *)array->mem + pos * array->sz;
}

int
arr_insert(struct arr *array, size_t pos, void *value)
{
	size_t len = arr_length(array);
	size_t sz = array->sz;
	void *insert, *elem;

	assert(array->init == 1);
	assert(pos <= arr_length(array));

	if (mem_grow(&array->mem, sz) < 0)
		return -1;

	insert = (char *)array->mem + pos * sz;
	elem = (char *)array->mem + (len - 1) * sz;
	for (; elem >= insert; elem = (char *)elem - sz)
		memcpy((char *)elem + sz, elem, sz);
	memcpy(insert, value, sz);
	return 0;
}

int
arr_append(struct arr *array, void *value)
{
	assert(array->init == 1);

	return arr_insert(array, arr_length(array), value);
}

int
arr_delete(struct arr *array, size_t pos)
{
	size_t sz = array->sz;
	size_t len = arr_length(array->mem);
	char *delete, *end, *elem;

	assert(array->init == 1);
	assert(pos < arr_length(array));

	delete = (char *)array->mem + pos * sz;
	end = (char *)array->mem + len * sz;
	for (elem = delete; elem < end; elem = (char *)elem + sz)
		memcpy(elem, (char *)elem + sz, sz);
	return mem_shrink(array->mem, array->sz);
}

int
arr_init(struct arr *array, size_t sz, struct mem_pool *pool)
{
	assert(array->init == 0);

	array->init = 1;
	array->sz = sz;
	array->mem = mem_alloc(pool, 0);
	if (array->mem == NULL)
		return -1;
	return 0;
}
