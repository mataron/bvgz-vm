#ifndef _BVGZ_HASH_MAP_H
#define _BVGZ_HASH_MAP_H

#include <stdint.h>


// impl from: https://github.com/petewarden/c_hashmap
#define MAP_MISSING -3  /* No such element */
#define MAP_FULL -2 	/* Hashmap is full */
#define MAP_OMEM -1 	/* Out of Memory */
#define MAP_OK 0 	    /* OK */


typedef void* hashmap_t;

hashmap_t hmap_create();
void hmap_destroy(hashmap_t map);

int32_t hmap_size(hashmap_t map);

int hmap_put(hashmap_t map, char* key, void* value);
int hmap_get(hashmap_t map, char* key, void** value);
int hmap_remove(hashmap_t map, char* key);

#endif
