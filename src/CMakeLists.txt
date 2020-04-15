add_library(topo SHARED topologies.cpp)
add_definitions(-DNUM_INITIATORS=${NUM_INITIATORS} -DRANDOM_SEED=${RANDOM_SEED})

