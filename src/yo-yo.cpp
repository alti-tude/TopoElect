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
        topo::log("INIT", buffer, i, true);

    }
    for(int i = 0 ; i < topo::num_neighbours ; i++) {
        std::vector<ll> recv_buf = topo::recv_from_neighbour(MPI_ANY_SOURCE, TAGS_INIT, true);
        ll source_idx = recv_buf[recv_buf.size() - 1];
        recv_buf.pop_back();
        topo::log("INIT", recv_buf, source_idx, false);

        InitMsg tmp_init = topo::unmarshal<InitMsg>(recv_buf);

        if(tmp_init.rank < topo::rank)
            incoming_edges.push_back(source_idx);
        else
            outgoing_edges.push_back(source_idx);
    }
    // END SETUP PHASE

    bool is_alive = true;
    while(is_alive){
        
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
            if(outgoing_edges.size() == 0) {
                // WHEN ONLY ALIVE NODE IS THE SOURCE => LEADER.
                std::cout << topo::rank << " is the elected leader."  << std::endl;
                break;
            }
            for(auto i:outgoing_edges) {
                std::vector<ll> source_vector = topo::marshal<NodeRecv>(source_value);
                topo::send_to_neighbour(source_vector, i, YO_TAG);
                topo::log("YO-", source_vector, i, true);
            }
        }
        else {
            // SINK AND INTERNAL NODE COMMON FUNCTIONS
            for(auto i:incoming_edges) {
                std::vector<ll> tmp_vector = topo::recv_from_neighbour(MPI_ANY_SOURCE, YO_TAG, true);

                ll source_idx = tmp_vector[tmp_vector.size() - 1];
                tmp_vector.pop_back();
                topo::log("YO-", tmp_vector, source_idx, false);


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
                topo::log("YO-", tmp_vector, i, true);
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
            if(incoming_edges.size() == 1) {
                topo::send_to_neighbour(YES, incoming_edges[0], PRUNE_TAG);
                topo::log("PRUNE", YES, incoming_edges[0], true);
                is_alive = false;
                incoming_edges.erase(incoming_edges.begin());
            }
            else {
                // SINK STARTS PROPAGATION
                for(auto i:pruned_edges) {
                    topo::send_to_neighbour(YES, i, PRUNE_TAG);
                    topo::log("PRUNE", YES, i, true);
                }


                for(auto i:min_value_edges) {
                    topo::send_to_neighbour(YES, i, OY_TAG);
                    topo::log("-YO", YES, i, true);
                }

                for(auto i:non_min_value_edges) {
                    topo::send_to_neighbour(NO, i, OY_TAG);
                    topo::log("-YO", NO, i, true);

                    // INCOMING EDGE -> OUTGOING EDGE
                    inc_edges.erase(std::find(inc_edges.begin(), inc_edges.end(), i));
                    outgoing_edges.push_back(i);
                }
            }
        }
        else {
            // SOURCE AND INTERNAL NODE COMMON FUNCTIONS
            bool final_value = 1;
            ll len = outgoing_edges.size();
            std::vector<ll> tmp_incoming_edges;
            for(ll i = 0 ; i < len ; i++) {    
                std::vector<ll> recv_bool_v = topo::recv_from_neighbour(MPI_ANY_SOURCE, MPI_ANY_TAG, true, true);
                ll tag = recv_bool_v[recv_bool_v.size() - 1];
                recv_bool_v.pop_back();
                ll source_idx = recv_bool_v[recv_bool_v.size() - 1];
                recv_bool_v.pop_back();
                topo::log("-YO", recv_bool_v, source_idx, false);

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

            // SEND YES OR NO TO INCOMING EDGES - NONE IN CASE OF SOURCE
            // YES TO EDGES FROM WHICH MINIMUM IS OBTAINED
            if(final_value) {
                for(auto i:pruned_edges){
                    topo::send_to_neighbour(YES, i, PRUNE_TAG);
                    topo::log("PRUNE", YES, i, true);
                }

                for(auto i:min_value_edges) {
                    topo::send_to_neighbour(YES, i, OY_TAG);
                    topo::log("-YO", YES, i, true);
                }

                for(auto i:non_min_value_edges) {
                    topo::send_to_neighbour(NO, i, OY_TAG);
                    topo::log("-YO", NO, i, true);

                    // INCOMING EDGE -> OUTGOING EDGE
                    inc_edges.erase(std::find(inc_edges.begin(), inc_edges.end(), i));
                    outgoing_edges.push_back(i);
                }
            }
            else {
                // SEND NO TO ALL AND REVERSE ALL
                int len = incoming_edges.size();
                for(ll i = 0 ; i < len ; i++) {
                    topo::send_to_neighbour(NO, i, OY_TAG);
                    topo::log("-YO", NO, i, true);

                    // INCOMING EDGE -> OUTGOING EDGE
                    inc_edges.erase(std::find(inc_edges.begin(), inc_edges.end(), i));
                    outgoing_edges.push_back(i);
                }
            }

            // UPDATE INCOMING EDGES AFTER YES OR NO HAS BEEN SENT TO INCOMING EDGES
            inc_edges.insert(inc_edges.end(), tmp_incoming_edges.begin(), tmp_incoming_edges.end());
        }
        incoming_edges = inc_edges;
    }
}