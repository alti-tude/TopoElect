#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"

namespace topo{
    std::vector<std::vector<int> > adjacency_list;
    std::vector<std::vector<vals> > globals;

    int rank;
    int numprocs;

    void init(){
        for(int i=0;i<numprocs;i++) 
            adjacency_list.push_back( std::vector<int>() ); 
    }

    void make_ring(){
        for(int i=0;i<numprocs;i++){
            std::vector<int> v;
            int next = (i+1)%numprocs;

            v.push_back( next );
            adjacency_list.push_back(v);
        }
    }

    int make_global(std::vector<vals> buffer, bool is_root){

        if(is_root){
            std::vector<MPI_Request> reqs;
            std::vector<MPI_Status> stats;
            for(int i=0;i<numprocs;i++){
                if(i == rank) continue;

                MPI_Request req;
                MPI_Status status;
                MPI_Isend(&buffer[0], buffer.size(), MPI_LONG_LONG_INT, i, globals.size(), MPI_COMM_WORLD, &req);
                reqs.push_back(req);
                stats.push_back(status);
            }
            MPI_Waitall(reqs.size(), &reqs[0], &stats[0]);
        }
        else{
            int flag = 0;
            MPI_Status status;

            MPI_Probe(MPI_ANY_SOURCE, globals.size(), MPI_COMM_WORLD, &status);

            int source = status.MPI_SOURCE;
            int count;
            MPI_Get_count(&status, MPI_LONG_LONG_INT, &count);

            MPI_Request req;
            buffer.resize(count);
            MPI_Irecv(&buffer[0],count,MPI_LONG_LONG_INT,source,globals.size(),MPI_COMM_WORLD, &req);
            MPI_Wait(&req, &status);            
        }

        MPI_Barrier(MPI_COMM_WORLD);
        
        globals.push_back(buffer);
        return globals.size();
    }
}