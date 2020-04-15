#ifndef TOPOLOGIES_H
#define TOPOLOGIES_H

#include "mpi.h"
#include "vector"
#include "assert.h"

namespace topo{
    extern int TAGS_GATHER_NEIGHBOURS;
    extern int TAGS_REDUCE_NEIGHBOURS;
    extern int TAGS_CUSTOM_BASE;
    
    // extern std::vector<std::vector<int> > adjacency_list;
    // extern std::vector<int> neighbours;
    extern std::vector<std::vector<long long int> > globals;

    extern int rank;
    extern int numprocs;
    extern int num_neighbours;
    extern bool is_initiator;

    void init();
    void make_ring();
    int make_global(std::vector<long long int> v, bool is_root=false);

    std::vector<long long int> recv_from_neighbour(int idx=MPI_ANY_SOURCE, int tag=MPI_ANY_TAG, bool return_source = false, bool return_tag=false);
    void send_to_neighbour(std::vector<long long int>& buffer, int idx, int tag);

    void reduce_neighbours(long long int* val, long long int (*reduction)(long long int a, long long int b));
    std::vector<std::vector<long long int> > gather_neighbours(std::vector<long long int>& send_buffer);

    template<typename T>
    std::vector<long long int> marshal(T s){
        assert(sizeof(s)%sizeof(long long int) == 0);
        int max = sizeof(s)/sizeof(long long int);

        long long int* p = (long long int*)&s;
        std::vector<long long int> out;
        for(int i=0;i<max;i++){
            std::cout << *(p+i) << " ";
            out.push_back(*(p+i));
        }
        std::cout <<sizeof(s) << std::endl;

        return out;
    }

    template<typename T>
    T unmarshal(std::vector<long long int>& v){
        T s;
        assert(sizeof(s) == v.size()*sizeof(long long int));

        long long int* p = (long long int*)&s;
        for(int i=0;i<v.size();i++){
            *(p+i) = v[i];    
        }

        return s;
    }
}

#endif