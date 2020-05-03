

#include "mapreduce.hpp"


/*
 * ****************************************
 * MAP REDUCE FUNCTIONS
 * ****************************************
 */


MapReduce::MapReduce(MPI_Comm world_) {
    world = world_;
    MPI_Comm_rank(world, &rank);
    MPI_Comm_size(world, &w_size);

    const int total_items = 2;
    
    int block_lengths[2] = {1, 1};
    
    MPI_Datatype types[2] = {MPI_INT, MPI_DOUBLE};
    
    MPI_Aint offsets[2];
    offsets[0] = offsetof(kv_t, key);
    offsets[1] = offsetof(kv_t, value);

    MPI_Type_create_struct(total_items, block_lengths, offsets, types, &mpi_kv_t);
    MPI_Type_commit(&mpi_kv_t);

    MPI_Barrier(world);
}


void MapReduce::map(std::function<void(int, int, vector<kv_t> *, void *)> map_func, void * data) {
    map_func(rank, w_size, &kv_, data);
}


void MapReduce::aggregate(int max_key) {
    vector<kv_t> new_;

    kv_t temp_;
    int flag = w_size-1, req_index = 0;

    MPI_Request send_req[kv_.size() + flag];

    for(kv_t& item_: kv_)
        MPI_Isend(&item_, 1, mpi_kv_t, (item_.key * w_size)/(max_key+1), item_.key, world, send_req + req_index++);
    
    kv_t item_{-1, 0};
    
    for(int i=0; i<w_size; i++)
        if(i == rank) continue;
        else MPI_Isend(&item_, 1, mpi_kv_t, i, 0, world, send_req + req_index++);

    while(flag) {
        MPI_Recv(&temp_, 1, mpi_kv_t, MPI_ANY_SOURCE, MPI_ANY_TAG, world, NULL);
        if(temp_.key == -1) flag--;
        else new_.push_back(temp_);
    }

    MPI_Waitall(req_index, send_req, MPI_STATUS_IGNORE);

    kv_.clear();
    kv_ = new_;
    new_.clear();
}

void MapReduce::convert() {
    // Sorting Keys
    sort(kv_.begin(), kv_.end(), [](kv_t _1, kv_t _2) {return _1.key < _2.key;});
    // Accumalting duplicate keys
    int key = -1, size = 0;
    double test = 0;
    kmv_t temp_;
    for(kv_t item_: kv_) {
        if(key != item_.key) {
            key = temp_.key = item_.key;
            temp_.values.clear();
            kmv_.push_back(temp_);
            size++;
        }
        kmv_[size-1].values.push_back(item_.value);
    }
    // Clearning kv values
    kv_.clear();
}

void MapReduce::collate(int max_key) {
    aggregate(max_key);
    convert();
}


void MapReduce::reduce(std::function<void(int, vector<double>&, void *)> reduce_function, void * data) {
    for(kmv_t& item_: kmv_)
        reduce_function(item_.key, item_.values, data);    
}

void MapReduce::reset() {
    kmv_.clear();
    kv_.clear();
    MPI_Barrier(world);
}