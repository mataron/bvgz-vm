#ifndef _BVGZ_HASH_SET_H
#define _BVGZ_HASH_SET_H

#include "hashmap.h"

typedef hashmap_t hashset_t;

#define hset_create()           hmap_create()
#define hset_destroy(set)       hmap_destroy(set)

#define hset_size(set)          hmap_size(set)

#define hset_add(set, key)      hmap_put(set, key, NULL)
#define hset_contains(set, key) (hmap_get(set, key, NULL) == MAP_OK)
#define hset_remove(set, key)   hmap_remove(set, key)

#endif
