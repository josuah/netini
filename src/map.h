#ifndef MAP_H
#define MAP_H

#include <stddef.h>

#include "mem.h"
#include "arr.h"

struct map_entry {
	char *key;
	void *value;
};

struct map {
	int init;
	struct mem_pool *pool;
	struct arr array;
};

/** src/map.c **/
size_t map_length(struct map *map);
void * map_get(struct map *map, char *key);
struct map_entry * map_i(struct map *map, size_t i);
int map_set(struct map *map, char *key, void *value);
int map_delete(struct map *map, char *key);
int map_init(struct map *map, struct mem_pool *pool);

#endif
