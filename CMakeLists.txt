project(fasindex CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++0x -DFAS_USE_RT_LIB ")
add_executable(test main.cpp)