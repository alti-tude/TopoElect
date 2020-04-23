#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"
#include "assert.h"
#include "random"
#include "set"
#include "fstream"

namespace topo{
    std::vector<std::vector<long long int> > adjacency_list;
    std::vector<long long int> neighbours;
    std::vector<std::vector<long long int> > globals;

    long long int TAGS_GATHER_NEIGHBOURS = 101;
    long long int TAGS_REDUCE_NEIGHBOURS = 102;
    long long int TAGS_CUSTOM_BASE = 200;
    
    size_t bsend_buffer_size = 1024+2*MPI_BSEND_OVERHEAD;
    void* bsend_buffer;

    long long int rank;
    long long int numprocs;
    long long int num_neighbours;
    bool is_initiator;

    void init(){
        for(int i=0;i<numprocs;i++) 
            adjacency_list.push_back( std::vector<long long int>() ); 

        #ifdef NUM_INITIATORS
        srand(rank+RANDOM_SEED);
        if(rank==0) is_initiator = true;
        else if(rand()%(numprocs-1)<(NUM_INITIATORS-1)) is_initiator=true;
        #endif 
        
        bsend_buffer = malloc(bsend_buffer_size);
        MPI_Buffer_attach(bsend_buffer, bsend_buffer_size);
    }

    void finalise(){
        std::ofstream graph_file;
        graph_file.open("graph.txt", std::ios::out);
        graph_file << "";
        graph_file.close();
        MPI_Barrier(MPI_COMM_WORLD);

        graph_file.open("graph.txt", std::ios::app);
        graph_file << rank << (is_initiator? ",i: ": ",n: "); 
        for(auto it:neighbours) graph_file << it << " ";
        graph_file << std::endl; 
        graph_file.close();
    }

    void make_ring(){
        if(numprocs == 2){
            adjacency_list[0].push_back(1);
            adjacency_list[1].push_back(0);
        }
        else{
            for(int i=0;i<numprocs;i++){
                std::vector<long long int> v;
                long long int next = (i+1)%numprocs;

                adjacency_list[i].push_back(next);
                adjacency_list[next].push_back(i);
            }
        }

        neighbours = adjacency_list[rank];
        num_neighbours = neighbours.size();
    }
  
    void make_mesh(){
        for(int i=0;i<numprocs;i++){
            for(int j=i+1;j<numprocs;j++){
                adjacency_list[i].push_back(j);
                adjacency_list[j].push_back(i);
            }
        }

        neighbours = adjacency_list[rank];
        num_neighbours = neighbours.size();
    }
    
    void make_custom() {
        if(numprocs != 9){
            make_ring();
            return;
        }
        else{
            adjacency_list[0].push_back(5);
            adjacency_list[0].push_back(7);
            adjacency_list[1].push_back(7);
            adjacency_list[1].push_back(8);
            adjacency_list[2].push_back(3);
            adjacency_list[2].push_back(6);
            adjacency_list[3].push_back(2);
            adjacency_list[3].push_back(4);
            adjacency_list[4].push_back(3);
            adjacency_list[4].push_back(8);
            adjacency_list[4].push_back(6);
            adjacency_list[5].push_back(0);
            adjacency_list[5].push_back(7);
            adjacency_list[6].push_back(4);
            adjacency_list[6].push_back(2);
            adjacency_list[7].push_back(1);
            adjacency_list[7].push_back(0);
            adjacency_list[7].push_back(5);
            adjacency_list[8].push_back(1);
            adjacency_list[8].push_back(4);

            neighbours = adjacency_list[rank];
            num_neighbours = neighbours.size();
        }
    }
    
    void make_general_graph(){
        long long int conn_prob = numprocs;
    
        #ifdef CONNECTION_PROBABILITY
        conn_prob = numprocs + CONNECTION_PROBABILITY;
        #endif

        #ifdef RANDOM_SEED
        srand(RANDOM_SEED);
        #endif
        
        
        for(int i=0;i<numprocs;i++){
            if(i!=0) 
                adjacency_list[i].push_back(i-1),
                adjacency_list[i-1].push_back(i);
                
            for(int j=0;j<numprocs;j++){
                if(i==j) continue;
                if(rand()%conn_prob<=1) 
                    adjacency_list[j].push_back(i),
                    adjacency_list[i].push_back(j);
            }
        }

        for(int i=0;i<numprocs;i++){
            std::set<long long int> st(adjacency_list[i].begin(), adjacency_list[i].end());
            adjacency_list[i] = std::vector<long long int>(st.begin(), st.end());
        }

        neighbours = adjacency_list[rank];
        num_neighbours = neighbours.size();

    }

    std::vector<long long int> non_blocking_recv(long long int source, long long int tag, MPI_Status& status){

        std::vector<long long int> buffer;

        MPI_Probe(source, tag, MPI_COMM_WORLD, &status);
        source = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        int count;
        MPI_Get_count(&status, MPI_LONG_LONG_INT, &count);

        MPI_Request req;
        buffer.resize(count);
        MPI_Irecv(&buffer[0],count,MPI_LONG_LONG_INT,source,tag,MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &status);  

        return buffer;     
    }

    std::vector<long long int> blocking_recv(long long int source, long long int tag, MPI_Status& status){
        std::vector<long long int> buffer;

        MPI_Probe(source, tag, MPI_COMM_WORLD, &status);
        source = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        int count;
        MPI_Get_count(&status, MPI_LONG_LONG_INT, &count);

        buffer.resize(count);
        MPI_Recv(&buffer[0],count,MPI_LONG_LONG_INT,source,tag,MPI_COMM_WORLD, &status);
        // MPI_Wait(&req, &status);  

        return buffer;     
    }

    void send_neighbours(std::vector<long long int>& buffer, long long int tag, std::vector<bool>& send, std::vector<MPI_Request>& reqs, std::vector<MPI_Status>& stats){        
        for(auto it:neighbours){
            if(!send[it]) continue;
            
            MPI_Request req;
            MPI_Status status;

            MPI_Isend(&buffer[0], buffer.size(), MPI_LONG_LONG_INT,it, tag, MPI_COMM_WORLD, &req);
            reqs.push_back(req);
            stats.push_back(status);
        }
    }

    std::vector<long long int> recv_from_neighbour(long long int idx, long long int tag, bool return_source, bool return_tag){
        MPI_Status status;
        std::vector<long long int> buffer;
        if(idx == MPI_ANY_SOURCE) buffer = non_blocking_recv(MPI_ANY_SOURCE, tag, status);
        else{
            assert(idx < num_neighbours);
            buffer = non_blocking_recv(neighbours[idx], tag, status);
        }
        
        if(return_source) 
            for(int i=0;i<num_neighbours;i++)
                if(status.MPI_SOURCE==neighbours[i]){
                    buffer.push_back(i);
                    break;
                }

        if(return_tag) buffer.push_back(status.MPI_TAG);
        return buffer;
    }

    std::vector<long long int> blocking_recv_from_neighbour(long long int idx, long long int tag, bool return_source, bool return_tag){
        MPI_Status status;
        std::vector<long long int> buffer;
        if(idx == MPI_ANY_SOURCE) buffer = blocking_recv(MPI_ANY_SOURCE, tag, status);
        else{
            assert(idx < num_neighbours);
            buffer = blocking_recv(neighbours[idx], tag, status);
        }
        
        if(return_source) 
            for(int i=0;i<num_neighbours;i++)
                if(status.MPI_SOURCE==neighbours[i]){
                    buffer.push_back(i);
                    break;
                }

        if(return_tag) buffer.push_back(status.MPI_TAG);
        return buffer;
    }

    void send_to_neighbour(std::vector<long long int>& buffer, long long int idx, long long int tag){
        assert(idx<num_neighbours);
        MPI_Bsend(&buffer[0], buffer.size(), MPI_LONG_LONG_INT, neighbours[idx], tag,MPI_COMM_WORLD);
    }

    void blocking_send_to_neighbour(std::vector<long long int>& buffer, long long int idx, long long int tag){
        assert(idx<num_neighbours);
        MPI_Send(&buffer[0], buffer.size(), MPI_LONG_LONG_INT, neighbours[idx], tag, MPI_COMM_WORLD);
    }

    long long int make_global(std::vector<long long int> buffer, bool is_root){
        std::vector<bool> send(numprocs, 1);
        std::vector<MPI_Request> reqs;
        std::vector<MPI_Status> stats;
        MPI_Status status;
        long long int source = rank;
        
        if(!is_root){
            buffer = blocking_recv(MPI_ANY_SOURCE, globals.size(), status);
            source = status.MPI_SOURCE;
        }

        send_neighbours(buffer, globals.size(), send, reqs, stats);

        for(auto it:neighbours) 
            if(it!=source) blocking_recv(it, globals.size(), status);

        MPI_Waitall(reqs.size(), &reqs[0], &stats[0]);
        MPI_Barrier(MPI_COMM_WORLD);
        
        globals.push_back(buffer);
        return globals.size();
    }

    void reduce_neighbours(long long int* val, long long int (*reduction)(long long int a, long long int b)){
        std::vector<bool> send(numprocs, 1);
        std::vector<MPI_Request> reqs;
        std::vector<MPI_Status> stats;
        MPI_Status status;
        long long int source = rank;

        std::vector<long long int> buffer = {*val};
        send_neighbours(buffer, TAGS_REDUCE_NEIGHBOURS, send, reqs, stats);
        
        bool flag = 0;
        long long int value = *val;
        for(auto it:neighbours){
            buffer = blocking_recv(it, TAGS_REDUCE_NEIGHBOURS, status);
            value = reduction(value, buffer[0]);
        }

        *val = value;
    }

    std::vector<std::vector<long long int> > gather_neighbours(std::vector<long long int>& send_buffer){
        std::vector<bool> send(neighbours.size(), 1);
        std::vector<MPI_Request> reqs;
        std::vector<MPI_Status> stats;
        MPI_Status status;
        long long int source = rank;

        send_neighbours(send_buffer, TAGS_GATHER_NEIGHBOURS, send, reqs, stats);

        std::vector<std::vector<long long int> > recvd;

        for(auto it:neighbours){
            std::vector<long long int> recv_buffer;
            recv_buffer = blocking_recv(it, TAGS_GATHER_NEIGHBOURS, status);
            recvd.push_back(recv_buffer);
        }

        return recvd;
    }
}   