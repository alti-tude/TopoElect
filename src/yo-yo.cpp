#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"
#include <limits.h>
#include <algorithm>
#include <set>

#define ll long long int
#define YO_TAG 2625
#define OY_TAG 6969
#define PRUNE_TAG 1999

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
    ll recv_value;
};

struct BoolMsg {
    ll val;
};

void run(){

    NodeInfo node;

    InitMsg send_init_msg;
    std::vector<ll> buffer = topo::marshal(send_init_msg);

    std::vector<ll> incoming_edges, outgoing_edges; // VECTORS THAT DEFINE THE DAG
    
    // SETUP PHASE
    // OBJ -> TO ESTABLISH INCOMING AND OUTGOING EDGES FOR ALL NODES
    for(int i = 0 ; i < topo::num_neighbours ; i++) {
        topo::send_to_neighbour(buffer, i, TAGS_INIT);
    }
    for(int i = 0 ; i < topo::num_neighbours ; i++) {
        std::vector<ll> recv_buf = topo::recv_from_neighbour(MPI_ANY_SOURCE, TAGS_INIT, true);
        ll source_idx = recv_buf[recv_buf.size() - 1];
        recv_buf.pop_back();
        InitMsg tmp_init = topo::unmarshal<InitMsg>(recv_buf);

        if(tmp_init.rank < topo::rank)
            incoming_edges.push_back(source_idx);
        else
            outgoing_edges.push_back(source_idx);
    }
    // END SETUP PHASE
    
    // YO- PHASE
    // OBJ -> PROPOGATE THE SOURCE VALUE TO THE CONNECTED SINKS

    std::vector<std::pair<ll, ll>> received_values;
    std::vector<ll> min_value_edges, non_min_value_edges;
    // received_values -> holds all pairs (source_idx, value received)
    // min_value_edges -> only those edge pairs from which we received minimum value
    // non_min_value_edges -> received_values - min_value_edges

    NodeRecv source_value;              // Only source will send this value to neighbours
    source_value.recv_value = topo::rank;

    // Internal node variable initializations
    ll min_recvd = LLONG_MAX;
    std::vector<ll> inc_edges(incoming_edges), pruned_edges;
    std::set<ll> unique_values;

    if(incoming_edges.size() == 0) {
        // SOURCE
        for(auto i:outgoing_edges) {
            std::vector<ll> source_vector = topo::marshal<NodeRecv>(source_value);
            topo::send_to_neighbour(source_vector, i, YO_TAG);
        }
    }
    else {
        // SINK AND INTERNAL NODE COMMON FUNCTIONS
        for(auto i:incoming_edges) {
            std::vector<ll> tmp_vector = topo::blocking_recv_from_neighbour(MPI_ANY_SOURCE, YO_TAG, true);

            ll source_idx = tmp_vector[tmp_vector.size() - 1];
            tmp_vector.pop_back();

            NodeRecv tmp_value = topo::unmarshal<NodeRecv>(tmp_vector);

            if(unique_values.find(tmp_value.recv_value) != unique_values.end()) {
                inc_edges.erase(std::find(inc_edges.begin(), inc_edges.end(), source_idx));
                pruned_edges.push_back(source_idx);
                continue;
            }
            unique_values.insert(tmp_value.recv_value);

            received_values.push_back(std::make_pair(source_idx, tmp_value.recv_value));

            if(tmp_value.recv_value < min_recvd)
                min_recvd = tmp_value.recv_value;
        }
        incoming_edges = inc_edges;

        // CLASSIFY EDGES FROM WHERE MINIMUM WAS OBTAINED
        for(auto i = received_values.begin() ; i != received_values.end() ; i++) {
            if(min_recvd == i->second)
                min_value_edges.push_back(i->first);
            else
                non_min_value_edges.push_back(i->first);
        }        

        // PROPAGATE MINIMUM VALUE RECEIVED - NO OUTGOING EDGES IN CASE OF SINK
        for(auto i:outgoing_edges) {
            NodeRecv tmp_send;
            tmp_send.recv_value = min_recvd;
            std::vector<ll> tmp_vector = topo::marshal(tmp_send);
            topo::send_to_neighbour(tmp_vector, i, YO_TAG);
        }
    }
    // END YO- PHASE

    // -YO PHASE
    // OBJ -> TO SEND YES AND NO MESSAGES

    // MESSAGES PROPAGATE IN OPPOSITE DIRECTIONS, HENCE
    // INCOMING_EDGES == OUTGOING_EDGES

    // Preparing yes and no messages
    BoolMsg yes, no;
    yes.val = 1;
    no.val = 0;

    std::vector<ll> YES, NO;
    YES = topo::marshal(yes);
    NO = topo::marshal(no);

    if(outgoing_edges.size() == 0) {
        // SINK STARTS PROPAGATION
        for(auto i:pruned_edges)
            topo::blocking_send_to_neighbour(YES, i, PRUNE_TAG);

        for(auto i:min_value_edges)
            topo::blocking_send_to_neighbour(YES, i, OY_TAG);

        for(auto i:non_min_value_edges) {
            topo::blocking_send_to_neighbour(NO, i, OY_TAG);

            // INCOMING EDGE -> OUTGOING EDGE
            incoming_edges.erase(std::find(incoming_edges.begin(), incoming_edges.end(), i));
            outgoing_edges.push_back(i);
        }
    }
    else {
        // SOURCE AND INTERNAL NODE COMMON FUNCTIONS
        bool final_value = 1;
        ll len = outgoing_edges.size();
        std::vector<ll> tmp_incoming_edges;
        for(ll i = 0 ; i < len ; i++) {    
            std::vector<ll> recv_bool_v = topo::blocking_recv_from_neighbour(MPI_ANY_SOURCE, MPI_ANY_TAG, true, true);
            ll tag = recv_bool_v[recv_bool_v.size() - 1];
            recv_bool_v.pop_back();
            ll source_idx = recv_bool_v[recv_bool_v.size() - 1];
            recv_bool_v.pop_back();
            // std::cout << recv_bool_v.size() << " size " << std::endl;

            if(tag == PRUNE_TAG) {
                outgoing_edges.erase(std::find(outgoing_edges.begin(), outgoing_edges.end(), source_idx));
                continue;
            }

            BoolMsg recv_bool = topo::unmarshal<BoolMsg>(recv_bool_v);
            final_value = final_value & (recv_bool.val % 2);

            // IF RECV VALUE IS NO THEN REVERSE EDGE
            if(!recv_bool.val) {
                // OUTGOING EDGE -> INCOMING EDGE
                outgoing_edges.erase(std::find(outgoing_edges.begin(), outgoing_edges.end(), source_idx));
                tmp_incoming_edges.push_back(source_idx);
            }
        }
        // std::cout << "rank : " << opo::rank << " final value " << final_value << std::endl;

        // SEND YES OR NO TO INCOMING EDGES - NONE IN CASE OF SOURCE
        // YES TO EDGES FROM WHICH MINIMUM IS OBTAINED
        if(final_value) {
            for(auto i:pruned_edges)
                topo::blocking_send_to_neighbour(YES, i, PRUNE_TAG);

            for(auto i:min_value_edges)
                topo::blocking_send_to_neighbour(YES, i, OY_TAG);

            for(auto i:non_min_value_edges) {
                topo::blocking_send_to_neighbour(NO, i, OY_TAG);

                // INCOMING EDGE -> OUTGOING EDGE
                incoming_edges.erase(std::find(incoming_edges.begin(), incoming_edges.end(), i));
                outgoing_edges.push_back(i);
            }
        }
        else {
            // SEND NO TO ALL AND REVERSE ALL
            int len = incoming_edges.size();
            for(ll i = 0 ; i < len ; i++) {
                topo::blocking_send_to_neighbour(NO, i, OY_TAG);

                // INCOMING EDGE -> OUTGOING EDGE
                incoming_edges.erase(std::find(incoming_edges.begin(), incoming_edges.end(), i));
                outgoing_edges.push_back(i);
            }
        }

        // UPDATE INCOMING EDGES AFTER YES OR NO HAS BEEN SENT TO INCOMING EDGES
        incoming_edges.insert(incoming_edges.end(), tmp_incoming_edges.begin(), tmp_incoming_edges.end());
    }

    std::cout << "Rank " << topo::rank << " incoming_edges : ";
    for(auto i:incoming_edges)
        std::cout << i << " ";
    std::cout << " || outgoing_edges : ";
    for(auto i:outgoing_edges)
        std::cout << i << " ";
    std::cout << std::endl;


}