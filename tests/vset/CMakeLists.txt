include_directories(. ../.. )

add_executable(test_vset1 test_vset1.cpp)
add_executable(vset_bench vset_bench.cpp)
add_executable(hitlist_bench hitlist_bench.cpp)

target_link_libraries(test_vset1 rt)
target_link_libraries(vset_bench rt)
target_link_libraries(hitlist_bench rt)



