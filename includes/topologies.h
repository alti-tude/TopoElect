#ifndef TOPOLOGIES_H
#define TOPOLOGIES_H

#include "mpi.h"
#include "vector"

namespace topo{
    union vals
    {
        long long int ival;
        double dval;
    };

    extern std::vector<std::vector<int> > adjacency_list;
    extern std::vector<int> neighbours;
    extern std::vector<std::vector<vals> > globals;

    extern int rank;
    extern int numprocs;

    void init();
    void make_ring();
    int make_global(std::vector<vals> v, bool is_root=false);

}

#endif