

#include "mapreduce.hpp"


/*
 * ****************************************
 * MAP REDUCE FUNCTIONS
 * ****************************************
 */


MapReduce::MapReduce(MPI_Comm world) {
    MPI_Comm_rank(world, &rank);
    MPI_Comm_size(world, &w_size);
}


int MapReduce::map(function<void(int, int, KeyValue *, void *)> map_func, void * data) {
    map_func(rank, w_size, &kv_, data);
}


void MapReduce::collate() {

}


int MapReduce::reduce(function<void(int, double *, int, void *)> reduce_function, void * data) {
    
}


/*
 * ****************************************
 * KEY VALUE FUNCTIONS
 * ****************************************
 */

void KeyValue::add(int key, double value) {
    data_.push_back({key, value});
}

void KeyValue::reset() {
    data_.clear();
}


/*
 * ****************************************
 * KEY MULTI-VALUE FUNCTIONS
 * ****************************************
 */