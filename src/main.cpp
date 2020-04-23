#include "topologies.h"

#include "mpi.h"
#include "iostream"
#include "unistd.h"

#include "test.h"

void debug(){
    if(topo::rank==0){
        volatile int i=0;
        std::cerr << "wating for debugger pid=" << getpid() << std::endl;

        while(i==0);
    }
}

int main(int argc, char* argv[]){
    MPI_Init( &argc, &argv );
    int rank, numprocs;
    
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
    topo::rank = rank;
    topo::numprocs = numprocs;

    #ifdef DEBUG
    debug();
    #endif
    
    double tbeg = MPI_Wtime();
    
    MPI_Barrier( MPI_COMM_WORLD );

    topo::init();

    #ifdef RING
    topo::make_ring();
    std::cout << "RING initialised by " << topo::rank << std::endl;
    #endif
    #ifdef MESH
    topo::make_mesh();
    std::cout << "MESH initialised by " << topo::rank << std::endl;
    #endif
    #ifdef CUSTOM
    topo::make_custom();
    // std::cout << "Custom initialised" << std::endl;
    #endif

    run();
    
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank==0){
        double time = MPI_Wtime()-tbeg;
        std::cerr << "Time taken: " << time << std::endl;
    }

    MPI_Finalize();
}