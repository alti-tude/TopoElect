#include "topologies.h"

#include "mpi.h"
#include "iostream"

#include "test.h"

int main(int argc, char* argv[]){
    MPI_Init( &argc, &argv );
    int rank, numprocs;
    
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
    topo::rank = rank;
    topo::numprocs = numprocs;

    MPI_Barrier( MPI_COMM_WORLD );

    topo::init();
    run();
    

    MPI_Finalize();
}