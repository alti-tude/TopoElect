#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"

void run(){
    for(int i=0;i<topo::numprocs;i++){
        for(int j=0;j<topo::numprocs;j++){
            if(i != j and topo::rank==j) {
                topo::make_global(std::vector<topo::vals>());
            }
            if(i==j and topo::rank==i){
                std::vector<topo::vals> v = {{.dval=1},{.dval=i+0.0},{.dval=i+2.0},{.dval=1}};
                topo::make_global(v, true);
            }
        }
    }

    if(topo::rank==0){
        for(auto it:topo::globals){
            for(auto jt:it){
                std::cout << jt.dval << " ";
            }
            std::cout << std::endl;
        }
    }
}