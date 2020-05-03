

#include "include.hpp"


typedef struct kv_s {
        int key;
        double value;
} kv_t;


typedef struct kmv_s {
        int key;
        vector<double> values;
} kmv_t;


class MapReduce {

private:
    int rank, w_size;

    vector<kmv_t> kmv_;

    vector<kv_t> kv_; 

    MPI_Datatype mpi_kv_t;
    MPI_Comm world;


public:

    MapReduce(MPI_Comm);

    void map(std::function<void(int, int, vector<kv_t> *, void *)>, void *);

    void collate(int);

    void aggregate(int);

    void convert();

    void reduce(std::function<void(int, vector<double>&, void *)>, void *);

    void reset();
};