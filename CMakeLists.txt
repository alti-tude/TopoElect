cmake_minimum_required(VERSION 3.07)
project(ds_project)

find_package(MPI REQUIRED) 
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_subdirectory(includes)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

add_subdirectory(src)
add_executable(main src/main.cpp src/test.cpp)
target_link_libraries(main ${MPI_CXX_LIBRARIES} topo)

