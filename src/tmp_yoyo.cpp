#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"
#include <limits.h>

#define ll long long int
#define YO_TAG 2625
#define OY_TAG 6969

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

void print_vector(std::vector<std::pair<ll, ll>> p) {
    for(auto i:p) std::cout << i.second << " ";
}

struct NodeRecv {
    ll idx;
    ll rank;
};

struct BoolMsg {
    bool val;
};

void run(){

    NodeInfo node;

    InitMsg send_init_msg;
    std::vector<long long int> buffer = topo::marshal(send_init_msg);
    long long int source_idx, tag;

    std::vector<std::pair<ll, ll>> incoming_edges;
    std::vector<std::pair<ll, ll>> outgoing_edges;


    // SETUP PHASE BEGINS
    for(int i=0;i<topo::num_neighbours;i++) topo::send_to_neighbour(buffer, i, TAGS_INIT);
    std::vector<std::pair<long long int, long long int>> edges;

    for(int i = 0 ; i < topo::num_neighbours ; i++) {
        std::vector<long long int> recv_buffer, msg_buffer;
        recv_buffer = topo::recv_from_neighbour(MPI_ANY_SOURCE, MPI_ANY_TAG, true, true);
        for(int j = 0 ; j < recv_buffer.size() - 2; j++) msg_buffer.push_back(recv_buffer[j]);
        source_idx = recv_buffer[recv_buffer.size() - 2];
        tag = recv_buffer[recv_buffer.size() - 1]; 
        InitMsg recvd_msg = topo::unmarshal<InitMsg>(msg_buffer);
        // std::cout << "Received value : " << recvd_msg.rank <<" from " << source_idx << std::endl;
        if(recvd_msg.rank < topo::rank) {
            // Edge should be from source node to me
            incoming_edges.push_back(std::make_pair(source_idx, recvd_msg.rank));
        }
        else {
            // Edge should be from me to source node
            outgoing_edges.push_back(std::make_pair(source_idx, recvd_msg.rank));
        }
    }

    // DEBUG CODE
    // ===================
    // std::cout << "Rank " << topo::rank << " incoming_edges : ";
    // for(auto i:incoming_edges)
    //     std::cout << i << " ";
    // std::cout << " || outgoing_edges : ";
    // for(auto i:outgoing_edges)
    //     std::cout << i << " ";
    // std::cout << std::endl;
    // ===================

    // Testing graph made
    // std::cout << "rank : " << topo::rank;
    // std::cout << " outgoing_edges : " ;
    // print_vector(outgoing_edges);
    // std::cout << " incoming_edges : ";
    // print_vector(incoming_edges);
    // std::cout << std::endl;
    // SETUP PHASE ENDS

    // YO- PHASE
    std::vector<NodeRecv> received_values;
    NodeRecv x;
    std::vector<ll> send_yes_to, send_no_to;
    ll min_recd;

    if(incoming_edges.size() == 0) {
        // SOURCE
        NodeRecv x;
        x.idx = -1;
        x.rank = topo::rank;
        std::vector<ll> b = topo::marshal<NodeRecv>(x);
        for(auto p:outgoing_edges){
            topo::send_to_neighbour(b, p.first, YO_TAG);
        }
    }
    else if(outgoing_edges.size() == 0) {
        // SINK
        min_recd = LLONG_MAX;
        for(int i = 0 ; i < incoming_edges.size() ; i++) {
            std::vector<ll> recv_buffer, msg_buffer;
            recv_buffer = topo::recv_from_neighbour(MPI_ANY_SOURCE, YO_TAG, true, true);
            for(int j = 0 ; j < recv_buffer.size() - 2; j++) msg_buffer.push_back(recv_buffer[j]);
            source_idx = recv_buffer[recv_buffer.size() - 2];

            tag = recv_buffer[recv_buffer.size() - 1]; 
            NodeRecv recvd_msg = topo::unmarshal<NodeRecv>(msg_buffer);
            std::cout << "Sink received value " << recvd_msg.rank << " from " <<  source_idx << std::endl;
            NodeRecv tmp;
            tmp.idx = source_idx;
            tmp.rank = recvd_msg.rank;
            received_values.push_back(tmp);


            //COMPUTE MINIMUM - DONE
            if(recvd_msg.rank < min_recd) {
                min_recd = recvd_msg.rank;
            }
        }

        x.idx = -1;
        x.rank = min_recd;

        
        for(auto q:received_values){
            if(q.rank == min_recd) {
                send_yes_to.push_back(q.idx);
                std::cout << "Will send yes to : " << q.idx << std::endl;
            }
            else {
                send_no_to.push_back(q.idx);
            }
        }
    }
    else {
        // INTERNAL NODE
        ll min_recd = LLONG_MAX;
        for(int i = 0 ; i < incoming_edges.size() ; i++) {
            std::vector<long long int> recv_buffer, msg_buffer;
            recv_buffer = topo::recv_from_neighbour(MPI_ANY_SOURCE, YO_TAG, true, true);
            for(int j = 0 ; j < recv_buffer.size() - 2; j++) msg_buffer.push_back(recv_buffer[j]);
            source_idx = recv_buffer[recv_buffer.size() - 2];
            tag = recv_buffer[recv_buffer.size() - 1]; 
            NodeRecv recvd_msg = topo::unmarshal<NodeRecv>(msg_buffer);
            NodeRecv tmp;
            tmp.idx = source_idx;
            tmp.rank = recvd_msg.rank;
            received_values.push_back(tmp);

            if(recvd_msg.rank < min_recd) {
                min_recd = recvd_msg.rank;
            }

        }

        x.idx = -1;
        x.rank = min_recd;
        std::vector<ll> to_send = topo::marshal(x);
        // SEND TO OUTGOING EDGE
        for(auto k:outgoing_edges) {
            topo::send_to_neighbour(to_send, k.first, YO_TAG);
        }
    }

    // END OF YO- PHASE

    // -YO PHASE
    // bool YES = 1, NO = 0;
    BoolMsg yes, no;
    yes.val = 1;
    no.val = 0;
    std::vector<ll> YES = topo::marshal(yes);
    std::vector<ll> NO = topo::marshal(no);

    std::pair<ll, ll> tmp_pair;
    bool val_send = 1;  
    if(outgoing_edges.size() == 0) {
        // SINK SENDS YES TO EDGES 
        for(auto i:send_yes_to) {
            topo::send_to_neighbour(YES, i, OY_TAG);
        }
        for(auto i:send_no_to) {
            topo::send_to_neighbour(NO, i, OY_TAG);

            // REVERSE THE DIRECTION OF EDGE
            // By removing it from incoming edges and adding it to outgoing edge
            for(auto it = incoming_edges.begin() ; it != incoming_edges.end() ; it++) {
                if(it->first == source_idx) {
                    tmp_pair = *it;
                    outgoing_edges.push_back(tmp_pair);
                    incoming_edges.erase(it);
                    break;
                }
            }
            // incoming_edges.erase(std::find(incoming_edges.begin(), incoming_edges.end(), tmp_pair), incoming_edges.end());
        }
    }
    else if(incoming_edges.size() == 0) {
        // SOURCE RECEIVES YES OR NO
        for(auto j:outgoing_edges) {
            std::vector<ll> p = topo::recv_from_neighbour(MPI_ANY_SOURCE, OY_TAG, true);
            source_idx = p[p.size() - 1];
            p.pop_back();
            BoolMsg recv = topo::unmarshal<BoolMsg>(p);
            val_send &= recv.val;
            if(recv.val == 0) {
                for(auto it = incoming_edges.begin() ; it != incoming_edges.end() ; it++) {
                    if(it->first == source_idx) {
                        tmp_pair = *it;
                        outgoing_edges.push_back(tmp_pair);
                        incoming_edges.erase(it);
                        break;
                    }
                }
                // incoming_edges.erase(std::remove(incoming_edges.begin(), incoming_edges.end(), tmp_pair), incoming_edges.end());
            }
        }
    }
    else {
        // INTERNAL NODE
        for(auto i:outgoing_edges) {
            std::vector<ll> p = topo::recv_from_neighbour(MPI_ANY_SOURCE, OY_TAG, true);
            source_idx = p[p.size() - 1];
            p.pop_back();
            BoolMsg recv = topo::unmarshal<BoolMsg>(p);
            val_send &= recv.val;
            if(recv.val == 0) {
                // REVERSE THE DIRECTION OF EDGE
                // By removing it from incoming edges and adding it to outgoing edge
                for(auto it = incoming_edges.begin() ; it != incoming_edges.end() ; it++) {
                    if(it->first == source_idx) {
                        tmp_pair = *it;
                        outgoing_edges.push_back(tmp_pair);
                        incoming_edges.erase(it);
                        break;
                    }
                }
                // incoming_edges.erase(std::remove(incoming_edges.begin(), incoming_edges.end(), tmp_pair), incoming_edges.end());
            }
        }

        // IF IT RECEIVED YES FROM ALL
        // val_send will be 1
        // YES CASE
        if(val_send == 1) {
            std::vector<ll> send_yes_to, send_no_to;
            for(auto k:received_values) {
                if(k.rank == min_recd) {
                    topo::send_to_neighbour(YES, k.idx, OY_TAG);
                }
                else {
                    topo::send_to_neighbour(NO, k.idx, OY_TAG);
                }
            }
        }
        else {
            for(auto k:incoming_edges) {
                topo::send_to_neighbour(NO, k.first, OY_TAG);
            }
        }

    }

    // Testing graph made
    std::cout << "rank : " << topo::rank;
    std::cout << " outgoing_edges : " ;
    print_vector(outgoing_edges);
    std::cout << " incoming_edges : ";
    print_vector(incoming_edges);
    std::cout << std::endl;

    // while(true){
        // std::vector<long long int> recvd_msg = topo::recv_from_neighbour(MPI_ANY_SOURCE, MPI_ANY_TAG, true, true);
        // std::vector<long long int> msg_buffer;
        // for(int i=0;i<recvd_msg.size()-2;i++) msg_buffer.push_back(recvd_msg[i]);
        // long long int source_idx = recvd_msg[recvd_msg.size()-2];
        // long long int tag = recvd_msg[recvd_msg.size()-1];

        // if(tag == TAGS_INIT and !node.is_awake){
        //     node.is_awake = true;
        //     InitMsg send_init_msg;
        //     std::vector<long long int> buffer = topo::marshal(send_init_msg);

        //     for(int i=0;i<topo::num_neighbours;i++) topo::send_to_neighbour(buffer, i, TAGS_INIT);
        // }
        // if(tag == TAGS_INIT){
        //     InitMsg recvd_init_msg = topo::unmarshal<InitMsg>(msg_buffer);
        //     //process the value recvd from the neighbour.
        // }
    // }
}