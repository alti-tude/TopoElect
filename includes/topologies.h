#ifndef TOPOLOGIES_H
#define TOPOLOGIES_H

#include "mpi.h"
#include "vector"
#include "assert.h"
#include "string"
#include "sstream"
#include "fstream"

namespace topo{
    extern long long int TAGS_GATHER_NEIGHBOURS;
    extern long long int TAGS_REDUCE_NEIGHBOURS;
    extern long long int TAGS_CUSTOM_BASE;
    
    // extern std::vector<std::vector<int> > adjacency_list;
    extern std::vector<long long int> neighbours;
    extern std::vector<std::vector<long long int> > globals;

    extern long long int rank;
    extern long long int numprocs;
    extern long long int num_neighbours;
    extern bool is_initiator;

    void init();
    void finalise();

    void make_ring();
    void make_mesh();
    void make_general_graph();

    long long int make_global(std::vector<long long int> v, bool is_root=false);

    std::vector<long long int> recv_from_neighbour(long long int idx=MPI_ANY_SOURCE, long long int tag=MPI_ANY_TAG, bool return_source = false, bool return_tag=false);
    void send_to_neighbour(std::vector<long long int>& buffer, long long int idx, long long int tag);

    void reduce_neighbours(long long int* val, long long int (*reduction)(long long int a, long long int b));
    std::vector<std::vector<long long int> > gather_neighbours(std::vector<long long int>& send_buffer);

    template<typename T>
    std::vector<long long int> marshal(T s){
        assert(sizeof(s)%sizeof(long long int) == 0);
        int max = sizeof(s)/sizeof(long long int);

        long long int* p = (long long int*)&s;
        std::vector<long long int> out;
        for(int i=0;i<max;i++){
            out.push_back(*(p+i));
        }

        return out;
    }

    template<typename T>
    T unmarshal(std::vector<long long int>& v){
        T s;
        assert(sizeof(s) == v.size()*sizeof(long long int));

        long long int* p = (long long int*)&s;
        for(int i=0;i<v.size();i++){
            *(p+i) = v[i];    
        }

        return s;
    }

    template<class T>
    inline void log(std::string msg_name, T msg, long long int to_from, bool send){
        std::stringstream ss;
        if(send) ss << rank << " sent to " << to_from <<" (";
        else ss << rank << " recieved from " << to_from <<" (";
        ss << msg_name << "): "; 
        
        std::vector<long long int> buffer = marshal(msg);
        for(auto it:buffer) ss << it << " ";
        ss << "\n";

        std::stringstream filename;
        filename << rank << "_msg_trace.txt";

        std::ofstream file;
        file.open(filename.str(), std::ios::app);
        file << ss.str();
        file.close();
    }

    template<>
    inline void log<std::vector<long long int> > (std::string msg_name, std::vector<long long int> msg, long long int to_from, bool send){
        std::stringstream ss;
        if(send) ss << rank << " sent to " << neighbours[to_from] <<" (";
        else ss << rank << " recieved from " << neighbours[to_from]
         <<" (";
        ss << msg_name << "): "; 
        
        for(auto it:msg) ss << it << " ";
        ss << "\n";

        std::stringstream filename;
        filename << rank << "_msg_trace.txt";

        std::ofstream file;
        file.open(filename.str(), std::ios::app);
        file << ss.str();
        file.close();
    }
}

#endif