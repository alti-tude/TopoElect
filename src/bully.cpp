#include "vector"
#include "bitset"
#include "iostream"
#include "mpi.h"

#include "topologies.h"
#include "bully/msgs.h"

long long int Min(long long int a, long long int b){
    return std::min(a,b);
}

std::vector<long long int> getMsgBuffer(std::vector<long long int>& v, bool return_source, bool return_tag){
    std::vector<long long int> msg_buffer;
    int end = v.size();
    if(return_source) end--;
    if(return_tag) end--;

    for(int i=0;i<end;i++) msg_buffer.push_back(v[i]);
    return msg_buffer;
}


struct NodeInfo{
    long long int father;
    long long int rank;
    long long int root;
    std::vector<bool> is_reject;

    NodeInfo():father(-1), rank(topo::rank), root(-1){
        is_reject.assign(topo::num_neighbours, 0);
    }

    void reset(){
        is_reject.assign(topo::num_neighbours, 0);
    }

};
std::ostream&  operator<<(std::ostream& out, const NodeInfo& node){
    out << node.rank << ": " << node.father << ", " << node.root << " ";
    for(int i=0;i<node.is_reject.size();i++) out << (int)node.is_reject[i];
    return out;
}



void run(){
    NodeInfo node;
    int Num_rejects = 0;

    if(topo::is_initiator) {
        node.root = topo::rank;
        // std::cout << "Setting root -- " << node.root << " " << node.rank << std::endl;
        node.father = -1;

        TestMsg test_msg(topo::rank);
        std::vector<long long int> buffer = topo::marshal(test_msg);

        for(int i=0;i<topo::num_neighbours;i++) {
            topo::send_to_neighbour(buffer, i, test_msg.tag);
        }
    }

    while(true)
    {
        std::vector<long long int> recvd_buffer = topo::recv_from_neighbour(MPI_ANY_SOURCE, MPI_ANY_TAG, true, true);
        std::vector<long long int> msg_buffer = getMsgBuffer(recvd_buffer, true, true);
        long long int tag = recvd_buffer[recvd_buffer.size()-1];
        long long int source_idx = recvd_buffer[recvd_buffer.size()-2];

        if(node.is_reject[source_idx]) continue;

        if(tag==TAGS_TEST)
        {
            TestMsg recvd_test_msg = topo::unmarshal<TestMsg>(msg_buffer);

            if(test_path[recvd_test_msg.rank][node.rank])
                continue;

            test_path[recvd_test_msg.rank][node.rank]=1;

            if(recvd_test_msg.rank < node.rank)
            {
                node.root = recvd_test_msg.root;
                node.father = source_idx;

                TestMsg send_test_msg(recvd_test_msg.root);
                for(int i=0;i<topo::num_neighbours;i++)
                {
                    std::vector<long long int> buffer = topo::marshal(send_test_msg);
                    topo::send_to_neighbour(buffer, i, TAGS_TEST);
                    topo::log("TEST", send_test_msg, i, true);
                }
            }

            else if(recvd_test_msg.rank > node.rank)
            {
                Reject send_reject_msg(node.rank);
                std::vector<long long int> buffer = topo::marshal(send_reject_msg);
                topo::send_to_neighbour(buffer, source_idx, TAGS_REJECT);
                topo::log("REJECT", send_reject_msg, source_idx, true);
            }
        }

        if(tag==TAGS_REJECT)
        {
            Reject recvd_reject_msg = topo::unmarshal<Reject>(msg_buffer);
            topo::log("REJECT", recvd_reject_msg, source_idx, false);
            Num_rejects -= 1;
        }

        if(tag==TAGS_VICTORY)
        {
            Victory recvd_victory_msg = topo::unmarshal<Victory>(msg_buffer);
            node.root=recvd_victory_msg.root;
            topo::log("VICTORY", recvd_victory_msg, source_idx, false);
            std::cout << "VICTORY: " << node.rank << ": " << node.root << ": is the root" << std::endl;

            for(int i=0;i<topo::num_neighbours;i++)
            {
                    Victory send_victory_msg(node.root);
                    std::vector<long long int> buffer = topo::marshal(send_victory_msg);
                    topo::send_to_neighbour(buffer, i, TAGS_VICTORY);
                    topo::log("VICTORY", send_victory_msg, i, true);
            }
    
            return;
        }

        if(Num_rejects == -topo::num_neighbours && !found)
        {
            std::cout << "VICTORY: " << node.rank << ": " << node.root << ": is the root\n";
            done[node.rank]=1;
            found=true;
            for(int i=0;i<topo::num_neighbours;i++)
            {
                Victory send_victory_msg(node.root);
                std::vector<long long int> buffer = topo::marshal(send_victory_msg);
                topo::send_to_neighbour(buffer, i, TAGS_VICTORY);
                topo::log("VICTORY", send_victory_msg, i, true);
            }

            return;
        }
        else if(done[node.rank])
            return;
    }
}