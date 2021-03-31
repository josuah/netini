#include "array.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"

size_t
array_length(struct array *arrayay)
{
	assert(arrayay->init == 1);

	return mem_length(arrayay->mem) / arrayay->sz;
}

void *
array_i(struct array *arrayay, size_t pos)
{
	assert(arrayay->init == 1);
	assert(pos < array_length(arrayay));

	return (char *)arrayay->mem + pos * arrayay->sz;
}

int
array_insert(struct array *arrayay, size_t pos, void *value)
{
	size_t len = array_length(arrayay);
	size_t sz = arrayay->sz;
	void *insert, *elem;

	assert(arrayay->init == 1);
	assert(pos <= array_length(arrayay));

	if (mem_grow(&arrayay->mem, sz) < 0)
		return -1;

	insert = (char *)arrayay->mem + pos * sz;
	elem = (char *)arrayay->mem + (len - 1) * sz;
	for (; elem >= insert; elem = (char *)elem - sz)
		memcpy((char *)elem + sz, elem, sz);
	memcpy(insert, value, sz);
	return 0;
}

int
array_append(struct array *arrayay, void *value)
{
	assert(arrayay->init == 1);

	return array_insert(arrayay, array_length(arrayay), value);
}

int
array_delete(struct array *arrayay, size_t pos)
{
	size_t sz = arrayay->sz;
	size_t len = array_length(arrayay->mem);
	char *delete, *end, *elem;

	assert(arrayay->init == 1);
	assert(pos < array_length(arrayay));

	delete = (char *)arrayay->mem + pos * sz;
	end = (char *)arrayay->mem + len * sz;
	for (elem = delete; elem < end; elem = (char *)elem + sz)
		memcpy(elem, (char *)elem + sz, sz);
	return mem_shrink(arrayay->mem, arrayay->sz);
}

int
array_init(struct array *arrayay, size_t sz, struct mem_pool *pool)
{
	assert(arrayay->init == 0);

	arrayay->init = 1;
	arrayay->sz = sz;
	arrayay->pool = pool;
	arrayay->mem = mem_alloc(pool, 0);
	if (arrayay->mem == NULL)
		return -1;
	return 0;
}
