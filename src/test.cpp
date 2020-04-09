#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"

void run(){
    topo::make_ring();
    MPI_Barrier(MPI_COMM_WORLD);

    // if(topo::rank==0){
    //     std::vector<topo::vals> v = {{.ival=1},{.ival=0},{.ival=2},{.ival=1}};
    //     topo::make_global(v, true);
    // }
    // else{
    //     topo::make_global(std::vector<topo::vals>(), false);
    // }

    for(int i=0;i<topo::numprocs;i++){
        for(int j=0;j<topo::numprocs;j++){
            if(i != j and topo::rank==j) {
                topo::make_global(std::vector<topo::vals>());
            }
            if(i==j and topo::rank==i){
                std::vector<topo::vals> v = {{.ival=1},{.ival=i+0},{.ival=i+2},{.ival=1}};
                topo::make_global(v, true);
            }
        }
    }

    if(topo::rank==0){
        for(auto it:topo::globals){
            for(auto jt:it){
                std::cout << jt.ival << " ";
            }
            std::cout << std::endl;
        }
    }
}