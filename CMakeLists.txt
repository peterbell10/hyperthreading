
cmake_minimum_required(VERSION 3.0.0)
project(hyperthreading CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENTIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

add_library(kernels kernels.cpp)

add_executable(add add.cpp)
target_link_libraries(add PRIVATE kernels)

add_executable(div div.cpp)
target_link_libraries(div PRIVATE kernels)

add_executable(mem mem.cpp)
target_link_libraries(mem PRIVATE kernels)

add_executable(div_mem div_mem.cpp)
target_link_libraries(div_mem PRIVATE kernels)

add_executable(div_add div_add.cpp)
target_link_libraries(div_add PRIVATE kernels)

add_executable(add_mem div_add.cpp)
target_link_libraries(add_mem PRIVATE kernels)

add_executable(nbody nbody.cpp)
target_link_libraries(nbody PRIVATE kernels)

add_executable(nbody_add nbody_add.cpp)
target_link_libraries(nbody_add PRIVATE kernels)

add_executable(nbody_mem nbody_mem.cpp)
target_link_libraries(nbody_mem PRIVATE kernels)
