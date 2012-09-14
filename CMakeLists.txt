cmake_minimum_required(VERSION 2.8)
project(fasindex CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -std=c++0x -DFAS_USE_RT_LIB ")
include_directories(. ../faslib2 )

#add_executable(test main.cpp)
#add_executable(test_chunk test_chunk.cpp)
#add_executable(test_vector test_vector.cpp)
#add_executable(test_sorted_vector test_sorted_vector.cpp)
add_executable(test_set test_set.cpp)
#add_executable(test_hitlist test_hitlist.cpp)


target_link_libraries(test_set rt)
#target_link_libraries(test rt)
#target_link_libraries(test_vector rt)
#target_link_libraries(test_sorted_vector rt)
#target_link_libraries(test_hitlist rt)