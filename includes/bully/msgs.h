#ifndef SHOUT_MSGS_H
#define SHOUT_MSGS_H
#include "topologies.h"
#include "iostream"
#include "string"

const long long int TAGS_TEST = topo::TAGS_CUSTOM_BASE + 1;
const long long int TAGS_ACK = topo::TAGS_CUSTOM_BASE + 2;
const long long int TAGS_REJECT = topo::TAGS_CUSTOM_BASE + 3;
const long long int TAGS_VICTORY = topo::TAGS_CUSTOM_BASE + 4;
long long int MAX_ID = 0;

struct Victory{
    long long int tag;
    long long int rank;
    long long int root;

    Victory():tag(TAGS_VICTORY) {}
    Victory(long long int root):tag(TAGS_VICTORY), rank(topo::rank), root(root) {}
};

struct TestMsg{
    long long int tag;
    long long int rank;
    long long int root;

    TestMsg():tag(TAGS_TEST) {}
    TestMsg(long long int root):tag(TAGS_TEST), rank(topo::rank), root(root) {}
};

struct Ack{
    long long int tag;
    long long int rank;
    long long int root;

    Ack():tag(TAGS_ACK) {}
    Ack(long long int root):tag(TAGS_ACK), rank(topo::rank), root(root){}
};

struct Reject{
    long long int tag;
    long long int rank;
    long long int root;

    Reject():tag(TAGS_REJECT) {}
    Reject(long long int root):tag(TAGS_REJECT), rank(topo::rank), root(root) {};
};

std::ostream& operator<<(std::ostream& out, TestMsg msg){
    out << "TEST: source(" << msg.rank << ")" << ", root(" << msg.root <<")";
    return out;
}

std::ostream& operator<<(std::ostream& out, Ack msg){
    out << "ACK: source(" << msg.rank << ")" << ", root(" << msg.root <<")";
    return out;
}

std::ostream& operator<<(std::ostream& out, Reject msg){
    out << "REJECT: source(" << msg.rank << ")" << ", root(" << msg.root <<")";
    return out;
}
#endif