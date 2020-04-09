#ifndef TOPOLOGIES_H
#define TOPOLOGIES_H

#include "mpi.h"
#include "vector"

namespace topo{
    extern int TAGS_GATHER_NEIGHBOURS;
    extern int TAGS_CUSTOM_BASE;
    
    extern std::vector<std::vector<int> > adjacency_list;
    extern std::vector<int> neighbours;
    extern std::vector<std::vector<long long int> > globals;

    extern int rank;
    extern int numprocs;

    void init();
    void make_ring();
    int make_global(std::vector<long long int> v, bool is_root=false);
}

#endif