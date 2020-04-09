#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"

namespace topo{
    std::vector<std::vector<int> > adjacency_list;
    std::vector<int> neighbours;
    std::vector<std::vector<vals> > globals;

    int rank;
    int numprocs;

    void init(){
        for(int i=0;i<numprocs;i++) 
            adjacency_list.push_back( std::vector<int>() ); 
    }

    void make_ring(){
        if(numprocs == 2){
            adjacency_list[0].push_back(1);
            adjacency_list[1].push_back(0);
        }
        else{
            for(int i=0;i<numprocs;i++){
                std::vector<int> v;
                int next = (i+1)%numprocs;

                adjacency_list[i].push_back(next);
                adjacency_list[next].push_back(i);
            }
        }

        neighbours = adjacency_list[rank];
    }


    std::vector<vals> blocking_recv(int source, int tag, MPI_Status& status){
        std::vector<vals> buffer;

        MPI_Probe(source, tag, MPI_COMM_WORLD, &status);

        source = status.MPI_SOURCE;
        int count;
        MPI_Get_count(&status, MPI_LONG_LONG_INT, &count);

        MPI_Request req;
        buffer.resize(count);
        MPI_Irecv(&buffer[0],count,MPI_LONG_LONG_INT,source,globals.size(),MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &status);  

        return buffer;     
    }

    void send_neighbours(std::vector<vals>& buffer, int tag, std::vector<bool>& send, std::vector<MPI_Request>& reqs, std::vector<MPI_Status>& stats){        
        for(int i=0;i<topo::neighbours.size();i++){
            if(!send[i]) continue;
            
            MPI_Request req;
            MPI_Status status;

            MPI_Isend(&buffer[0], buffer.size(), MPI_LONG_LONG_INT, topo::neighbours[i], tag, MPI_COMM_WORLD, &req);
            reqs.push_back(req);
            stats.push_back(status);
        }
    }

    int make_global(std::vector<vals> buffer, bool is_root){
        std::vector<bool> send(topo::numprocs, 1);
        std::vector<MPI_Request> reqs;
        std::vector<MPI_Status> stats;
        MPI_Status status;
        int source = topo::rank;
        
        if(!is_root){
            buffer = blocking_recv(MPI_ANY_SOURCE, globals.size(), status);
            source = status.MPI_SOURCE;
        }

        send_neighbours(buffer, globals.size(), send, reqs, stats);

        for(auto it:neighbours) {
            if(it!=source){
                blocking_recv(it, globals.size(), status);
            }
        }

        MPI_Waitall(reqs.size(), &reqs[0], &stats[0]);
        MPI_Barrier(MPI_COMM_WORLD);
        
        globals.push_back(buffer);
        return globals.size();
    }
}   