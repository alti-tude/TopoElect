#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"

const long long int TAGS_INIT = topo::TAGS_CUSTOM_BASE+1;

struct InitMsg{
    long long int rank;
    long long int tag;
    InitMsg(): rank(topo::rank), tag(TAGS_INIT) {}
};

struct NodeInfo{
    long long int rank;
    bool is_awake;
    NodeInfo(): rank(topo::rank), is_awake(0) {}
};

void run(){

    NodeInfo node;

    if(topo::is_initiator){
        InitMsg send_init_msg;
        std::vector<long long int> buffer = topo::marshal(send_init_msg);
        std::cout << topo::rank << " am initiator" << std::endl;

        for(int i=0;i<topo::num_neighbours;i++) topo::send_to_neighbour(buffer, i, TAGS_INIT);
    }

    // while(true){
        std::vector<long long int> recvd_msg = topo::recv_from_neighbour(MPI_ANY_SOURCE, MPI_ANY_TAG, true, true);
        std::vector<long long int> msg_buffer;
        for(int i=0;i<recvd_msg.size()-2;i++) msg_buffer.push_back(recvd_msg[i]);
        long long int source_idx = recvd_msg[recvd_msg.size()-2];
        long long int tag = recvd_msg[recvd_msg.size()-1];

        if(tag == TAGS_INIT and !node.is_awake){
            node.is_awake = true;
            InitMsg send_init_msg;
            std::vector<long long int> buffer = topo::marshal(send_init_msg);

            for(int i=0;i<topo::num_neighbours;i++) topo::send_to_neighbour(buffer, i, TAGS_INIT);
        }
        if(tag == TAGS_INIT){
            InitMsg recvd_init_msg = topo::unmarshal<InitMsg>(msg_buffer);
            //process the value recvd from the neighbour.
        }
    // }
}