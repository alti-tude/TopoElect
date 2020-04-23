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

struct TestMsg{
    long long int tag;
    long long int rank;
    long long int root;

    TestMsg(long long int root):tag(-1), rank(topo::rank), root(root) {}
};

void run(){
    // TestMsg T(2);

    // for(auto it:v) std::cout << it << " ";
    // std::cout << std::endl;
    // if(topo::rank==0) {
    //     std::vector<long long int> v = {1,2,3};
    //     for(int i=0;i<topo::num_neighbours;i++) {
    //         std::cout << "sent to " << i << "\n";
    //         topo::send_to_neighbour(v, i, 101);
    //     }
    // }
    // else{
    //     std::vector<long long int> v = topo::recv_from_neighbour(MPI_ANY_SOURCE, MPI_ANY_TAG, true, true);
    //     std::cout << topo::rank << ": ";
    //     for(auto it:v) std::cout << it << " ";
    //     std::cout << std::endl;
    // }
    // MPI_Barrier(MPI_COMM_WORLD);
    // if(topo::is_initiator) std::cout << "initiator" << " " << topo::rank << std::endl;
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
