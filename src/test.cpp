#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"

void run(){
    topo::make_ring();
    MPI_Barrier(MPI_COMM_WORLD);

    for(int i=0;i<topo::numprocs;i++){
        for(int j=0;j<topo::numprocs;j++){
            if(i != j and topo::rank==j) {
                topo::make_global(std::vector<long long int>());
            }
            if(i==j and topo::rank==i){
                std::vector<long long int> v = {1,i+0,i+2,1};
                topo::make_global(v, true);
            }
        }
    }

    if(topo::rank==0){
        for(auto it:topo::globals){
            for(auto jt:it){
                std::cout << jt << " ";
            }
            std::cout << std::endl;
        }
    }
}