#include "topologies.h"
#include "vector"
#include "mpi.h"
#include "iostream"
#include <stdlib.h>
#include <set>
#include <algorithm>

// #define Q 1603
#define Q_TAG 6969
#define YES_TAG 101010
#define RANK_RECV_TAG 9999
#define COUNT_RECV_TAG 9998
#define NEIGH_RECV_TAG 9997

const int Q = 1603;
const int YES = 8443;
const int NO = 9001;

void get_active(std::set<int>* neighbours, int total_no_of_neighbours) {
	int message;
	int deny = NO;
	int agree = YES;
	int apparent_neighbours = neighbours->size();
	MPI_Status status;
	while(true) {
		MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		switch(message) {
			case Q:
				MPI_Send(&deny, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD);
				break;
			case YES:
				neighbours->insert(status.MPI_SOURCE);
				apparent_neighbours += 1;
				if(total_no_of_neighbours == apparent_neighbours)
					return;
				break;
			case NO:
				apparent_neighbours += 1;
				if(total_no_of_neighbours == apparent_neighbours)
					return;
				break;
		}
	}
}

void run(){
    topo::make_ring();
    MPI_Barrier(MPI_COMM_WORLD);
    
    // SHOUT PROTOCOL BEGINS

    int initiator = rand() % topo::numprocs;
	std::set<int> neighbours;
	int msg = Q;


    if(topo::rank == initiator) {
    	for(auto neighbour:topo::neighbours) {
    		MPI_Send(&Q, 1, MPI_INT, neighbour, Q_TAG, MPI_COMM_WORLD);
    	}
    	get_active(&neighbours, topo::neighbours.size());
    }
    else {
    	int message;
    	MPI_Status status;
    	MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, Q_TAG, MPI_COMM_WORLD, &status);
    	int total_no_of_neighbours = topo::neighbours.size();
    	topo::neighbours.erase(std::remove(topo::neighbours.begin(), topo::neighbours.end(), status.MPI_SOURCE), topo::neighbours.end());
    	neighbours.insert(status.MPI_SOURCE);
    	MPI_Send(&YES, 1, MPI_INT, status.MPI_SOURCE, YES_TAG, MPI_COMM_WORLD);
    	if(neighbours.size() != total_no_of_neighbours) {
    		for(auto neighbour:topo::neighbours) {
    			MPI_Send(&Q, 1, MPI_INT, neighbour, Q_TAG, MPI_COMM_WORLD);
    		}
    		get_active(&neighbours, total_no_of_neighbours);
    	}
    }

    if(topo::rank == initiator) {
    	std::cout << initiator << " has been elected as the leader " << std::endl;
    }
}
