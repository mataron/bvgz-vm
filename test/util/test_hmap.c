#include "../tests.h"
#include "util/hashmap.h"


int main(int argc, char** argv)
{
    int ret;
    void* dt;

    hashmap_t map = hmap_create();

    assert(hmap_size(map) == 0);

    ret = hmap_put(map, "0", (void*)1);
    assert(MAP_OK == ret);
    assert(hmap_size(map) == 1);
    ret = hmap_put(map, "1", (void*)11);
    assert(MAP_OK == ret);
    assert(hmap_size(map) == 2);
    ret = hmap_put(map, "2", (void*)111);
    assert(MAP_OK == ret);
    assert(hmap_size(map) == 3);
    ret = hmap_put(map, "3", (void*)1111);
    assert(MAP_OK == ret);
    assert(hmap_size(map) == 4);
    ret = hmap_put(map, "4", (void*)11111);
    assert(MAP_OK == ret);
    assert(hmap_size(map) == 5);
    ret = hmap_put(map, "5", (void*)111111);
    assert(MAP_OK == ret);
    assert(hmap_size(map) == 6);

    ret = hmap_get(map, "0", &dt);
    assert(MAP_OK == ret);
    assert(dt == (void*)1);
    ret = hmap_get(map, "1", &dt);
    assert(MAP_OK == ret);
    assert(dt == (void*)11);

    ret = hmap_get(map, "---1", &dt);
    assert(MAP_MISSING == ret);

    ret = hmap_remove(map, "0");
    assert(MAP_OK == ret);
    assert(hmap_size(map) == 5);
    ret = hmap_remove(map, "0");
    assert(MAP_MISSING == ret);
    assert(hmap_size(map) == 5);

    ret = hmap_remove(map, "1");
    assert(MAP_OK == ret);
    assert(hmap_size(map) == 4);

    hmap_destroy(map);
    return 0;
}
