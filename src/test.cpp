#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"

long long int Min(long long int a, long long int b){
    return std::min(a,b);
}

void run(){
    topo::make_ring();
    MPI_Barrier(MPI_COMM_WORLD);

    if(topo::rank==0) {
        std::vector<long long int> v = {1,2,3};
        topo::send_to_neighbour(v, 0, 10);
        topo::send_to_neighbour(v, 1, 10);
    }
    else{
        std::vector<long long int> v = topo::recv_from_neighbour();
        std::cout << topo::rank << std::endl;
        for(auto it:v) std::cout << it << " ";
        std::cout << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(topo::is_initiator) std::cout << "asd" << " " << topo::rank << std::endl;
    // long long int val=topo::rank+1;
    // topo::reduce_neighbours(&val, &Min);

    // std::cout << topo::rank << " " << val << std::endl
    ; 
    // if(topo::rank==0){
    //     for(auto it:topo::globals){
    //         for(auto jt:it){
    //             std::cout << jt << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    // }
}