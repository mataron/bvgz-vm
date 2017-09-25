include_directories(.)

add_executable(test_list test_list.c)
target_link_libraries(test_list utils)
add_test(test_list test_list)

add_executable(test_hmap test_hmap.c)
target_link_libraries(test_hmap utils)
add_test(test_hmap test_hmap)
