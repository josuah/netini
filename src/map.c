#include "map.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "arr.h"
#include "log.h"

static int
map_cmp(void const *v1, void const *v2)
{
	struct map_entry const *e1 = v1, *e2 = v2;

	return strcmp(e1->key, e2->key);
}

size_t
map_length(struct map *map)
{
	assert(map->init == 1);

	return arr_length(&map->array);
}

void *
map_get(struct map *map, char *key)
{
	struct map_entry *entry, k = { .key = key };
	size_t sz, len = map_length(map);

	assert(map->init == 1);

	sz = sizeof(struct map_entry);
	entry = bsearch(&k, map->array.mem, len, sz, map_cmp);
	if (entry == NULL)
		return NULL;

	return entry->value;
}

struct map_entry *
map_i(struct map *map, size_t i)
{
	return arr_i(&map->array, i);
}

int
map_set(struct map *map, char *key, void *value)
{
	struct map_entry entry = { .key = key, .value = value };
	size_t i, len = map_length(map);

	assert(map->init == 1);
	assert(key != NULL);

	for (i = 0; i < len; i++) {
		struct map_entry *this = map_i(map, i);;
		int cmp = strcmp(key, this->key);

		if (cmp == 0) {
			*this = entry;
			return 0;
		}
		if (cmp < 0)
			break;
	}
	return arr_insert(&map->array, i, &entry);
}

int
map_delete(struct map *map, char *key)
{
	size_t i, len = map_length(map);

	assert(map->init == 1);

	for (i = 0; i < len; i++) {
		struct map_entry *entry = map_i(map, i);
		int cmp = strcmp(key, entry->key);

		if (cmp == 0)
			break;
		if (cmp < 0)
			return -1;
	}
	return arr_delete(&map->array, i);
}

int
map_init(struct map *map, struct mem_pool *pool)
{
	assert(map->init == 0);

	map->pool = pool;
	map->init = 1;
	return arr_init(&map->array, sizeof(struct map_entry), map->pool);
}
