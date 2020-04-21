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
    int acks_reqd = 0;

    if(topo::is_initiator) {
        node.root = topo::rank;
        node.father = -1;

        TestMsg test_msg(topo::rank);
        std::vector<long long int> buffer = topo::marshal(test_msg);

        for(int i=0;i<topo::num_neighbours;i++) {
            topo::send_to_neighbour(buffer, i, test_msg.tag);
            acks_reqd += 1;
        }
    }

    while(true){
        std::vector<long long int> recvd_buffer = topo::recv_from_neighbour(MPI_ANY_SOURCE, MPI_ANY_TAG, true, true);
        std::vector<long long int> msg_buffer = getMsgBuffer(recvd_buffer, true, true);
        long long int tag = recvd_buffer[recvd_buffer.size()-1];
        long long int source_idx = recvd_buffer[recvd_buffer.size()-2];

        if(node.is_reject[source_idx]) continue;

        if(tag==TAGS_TEST){
            TestMsg recvd_test_msg = topo::unmarshal<TestMsg>(msg_buffer);
            std::cout << recvd_test_msg << " " << node << std::endl;

            if(recvd_test_msg.root > node.root) {
                //my tree merges with the testing tree 
                node.root = recvd_test_msg.root;
                node.father = source_idx;
                acks_reqd = 0;

                //TODO: send test to all the non reject edges (except the one that you got it from)
                TestMsg send_test_msg(recvd_test_msg.root);
                for(int i=0;i<topo::num_neighbours;i++){
                    if(!node.is_reject[i] and node.father!=i) {
                        std::vector<long long int> buffer = topo::marshal(send_test_msg);
                        topo::send_to_neighbour(buffer, i, TAGS_TEST);
                        acks_reqd += 1;
                    }
                }
            }

            else if(recvd_test_msg.root == node.root){
                node.is_reject[source_idx] = 1;
                Reject send_reject_msg(node.root);
                std::vector<long long int> buffer = topo::marshal(send_reject_msg);
                topo::send_to_neighbour(buffer, source_idx, TAGS_REJECT);
                acks_reqd -=1;
            }
            //* if(recvd_test_msg.root < node.root) do nothing
        }

        if(tag==TAGS_REJECT){
            Reject recvd_reject_msg = topo::unmarshal<Reject>(msg_buffer);

            acks_reqd -= 1;
            node.is_reject[source_idx] = 1;            
            std::cout << recvd_reject_msg << " " << node << ": " << acks_reqd <<  std::endl;
        }

        if(tag==TAGS_ACK){
            Ack recvd_ack_msg = topo::unmarshal<Ack>(msg_buffer);

            if(recvd_ack_msg.root == node.root) acks_reqd -= 1;
            std::cout << recvd_ack_msg << " " << node << ": " << acks_reqd <<  std::endl;
        }

        if(tag==TAGS_VICTORY){
            Victory recvd_victory_msg = topo::unmarshal<Victory>(msg_buffer);

            std::cout << "VICTORY: " << node << ": " << recvd_victory_msg.root << ": is the root" << std::endl;
            for(int i=0;i<topo::num_neighbours;i++) {
                if(!node.is_reject[i] and node.father!=i){
                    Victory send_victory_msg(node.root);
                    std::vector<long long int> buffer = topo::marshal(send_victory_msg);
                    topo::send_to_neighbour(buffer, i, TAGS_VICTORY);
                }
            }

            return;
        }

        if(acks_reqd == 0 and node.root == node.rank){
            std::cout << "VICTORY: " << node << ": " << node.rank << ": is the root\n";
            for(int i=0;i<topo::num_neighbours;i++) {
                if(!node.is_reject[i] and node.father!=i){
                    Victory send_victory_msg(node.root);
                    std::vector<long long int> buffer = topo::marshal(send_victory_msg);
                    topo::send_to_neighbour(buffer, i, TAGS_VICTORY);
                }
            }

            return;
        }
        else if(acks_reqd == 0){
            Ack send_ack_msg(node.root);
            std::vector<long long int> buffer = topo::marshal(send_ack_msg);
            topo::send_to_neighbour(buffer, node.father, TAGS_ACK);
        }
    }
}