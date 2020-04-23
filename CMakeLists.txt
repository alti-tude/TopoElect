cmake_minimum_required(VERSION 3.07)
project(ds_project)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(NUM_INITIATORS 10) #rank 0 will always be an initiator
set(RANDOM_SEED 11)

set(TOPOLOGY "RING") #RING only for now
set(CONNECTION_PROBABILITY -2)

set(DEBUG "NDEBUG") #DEBUG / NDEBUG 

find_package(MPI REQUIRED) 
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_subdirectory(includes)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

add_subdirectory(src)
add_executable(main src/main.cpp src/test.cpp)
target_link_libraries(main ${MPI_CXX_LIBRARIES} topo)

add_executable(shout src/main.cpp src/shout.cpp)
target_link_libraries(shout ${MPI_CXX_LIBRARIES} topo)
add_definitions(-D${DEBUG} -D${TOPOLOGY} -DNUM_INITIATORS=${NUM_INITIATORS})