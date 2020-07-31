

#include "include.hpp"

/*
 * Key Value Structure
 */
typedef struct kv_s {
        int key; // key
        double value; // value
} kv_t;

/*
 * Key Multi-Value Structure
 */
typedef struct kmv_s {
        int key; // key
        vector<double> values; // multiple values
} kmv_t;



// MAP REDUCE CLASS
class MapReduce {

private:
    int rank, w_size; // MPI Parameters

    vector<kmv_t> kmv_; // KMV Container

    vector<kv_t> kv_; // KV Container

    MPI_Datatype mpi_kv_t; // MPI Datatype, for send over communications
    MPI_Comm world; // MPI comminication world


public:

    /*
     * MPI Constructor
     * Initialises the Map-Reduce state, and sets all data-type values.
     * @param (MPI_Comm): World over which communication is allowed.
     */
    MapReduce(MPI_Comm);

    /*
     * MAP Function
     * @param (std::function): The function over which the data is mapped 
     * @param (void *): The data pointer (to be passed to the inner called function)
     */
    void map(std::function<void(int, int, vector<kv_t> *, void *)>, void *);

    /*
     * Collate Function
     * Function, for aggregating similar keys at single cores and then converting them to KMV pairs.
     * @param (int): Max possible key value.
     */
    void collate(int);
    
    /*
     * Aggregate Function
     * Function, for aggregating similar keys at single cores.
     * @param (int): Max possible key value.
     */
    void aggregate(int);

    /*
     * Convert Function
     * Function, for converting KV pairs at each core to KMV pairs.
     */
    void convert();

    /*
     * REDUCE Function
     * @param (std::function): The function for reducing KMV pairs
     * @param (void *): The data pointer (to be passed to the inner called function)
     */
    void reduce(std::function<void(int, vector<double>&, void *)>, void *);

    /*
     * Resetting the complete MAPREDUCE object. (to be alled after a succesful cycle)
     */
    void reset();
};