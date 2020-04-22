cmake_minimum_required(VERSION 3.07)
project(ds_project)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(NUM_INITIATORS 8) #rank 0 will always be an initiator
set(RANDOM_SEED 10)
set(TOPOLOGY "RING") #RING or MESH
set(DEBUG "NDEBUG") #DEBUG / NDEBUG 

find_package(MPI REQUIRED) 
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_subdirectory(includes)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

add_subdirectory(src)
add_executable(main src/main.cpp src/test.cpp)
target_link_libraries(main ${MPI_CXX_LIBRARIES} topo)

add_executable(bully src/main.cpp src/bully.cpp)
target_link_libraries(bully ${MPI_CXX_LIBRARIES} topo)


add_definitions(-D${DEBUG} -D${TOPOLOGY})