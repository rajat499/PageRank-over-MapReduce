
#include "mapreduce.hpp"

#include "graph.hpp"


// Datasource Type
#define DATATYPE pair<double *, graph_item *>

// Algorithm Constants
double ALPHA = 0.85, LIMIT = 1e-13;
// Size containers
int graph_size = 0, list_size = 0;


/*
 * Euclidean difference b/w vectors
 * @param (_1, _2): vectors
 */
double difference(double *_1, double *_2) {
    double diff = 0;
    for(size_t i{}; i<graph_size; i++)
        diff += pow(_1[i] - _2[i], 2);
    return diff;
}


/*
 * Map Task Function.
 * Works on individual graph_item and extracts keys out.
 * 
 * @param (rank, w_size): MPI parameters
 * @param (vector<kv_t> *): Key-Value list pointer
 * @param (void *): Datasource pointer
 */
void calculate(int rank, int w_size, vector<kv_t> *kv, void *data) {
    // Extracting Rank Vector
    double *rank_ = ((DATATYPE *)data)->first;
    // Extracting Graph
    graph_item *graph_ = ((DATATYPE *)data)->second;

    // Local Parameters
    double v; int a, b; kv_t temp;

    // Specifying, task splitting.
    int _start = (rank*list_size)/w_size, _end = ((rank + 1)*list_size)/w_size;

    // Processing all graph_items (in range)
    for(int i=_start; i<_end; i++) {
        // Extracting Information
        v = graph_[i].first;
        a = graph_[i].second.first;
        b = graph_[i].second.second;
        // Accomodating rank
        v = rank_[a] * v;
        // Avoid 0 rank
        if(v == 0) continue;
        // Boundary cases
        if(b == -2) // DANGLING
            b = graph_size;
        else if(b == -1) // RANDOM JUMP
            b = graph_size + 1;
        // Corporating ALPHA
        v = (b > graph_size)?(v * (1-ALPHA)):(v * ALPHA);
        // Setting structure
        temp.key = b;
        temp.value = v;
        // Add to key-value
        kv->push_back(temp);
    }
}


/*
 * Reduce Task Function.
 * Works on Key-MultiValue pairs
 * 
 * @param (key): Key Value
 * @param (vector<double>&): Data Value Container
 * @param (void *): Datasource Pointer
 */
void collect(int key, vector<double>& values, void *data) {
    // Local container
    double total = 0;
    // Working with all values
    for(int i=0; i<values.size(); i++)
        total += values[i];
    // Extracing Rank Vector
    double *data_ = (double *)data;
    // Adding to vector (case wise)
    if(key >= graph_size)
        for(int i=0; i<graph_size; i++)
            data_[i] += total;
    else
        data_[key] += total;
}



// MAIN
int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    int self, num_proc;

    MPI_Comm_rank(MPI_COMM_WORLD,&self);
    MPI_Comm_size(MPI_COMM_WORLD,&num_proc);

    double start_time = MPI_Wtime(); // Timer

    // Reading Graph
    graph_item* graph;
    Graph(argv[1]).convert(&graph, &graph_size, &list_size);

    if(!self && *argv[2]=='1') // Logging
        cerr << "Graph Read. Size " << graph_size << ". Link Size " << list_size << ". Time " << (MPI_Wtime()-start_time) << "s.\n";

    // Creating rank containers
    double *rank = new double[graph_size](); rank[0] = 1;
    double *new_rank = new double[graph_size]();

    // Setting the general datatype
    DATATYPE data = {rank, graph};

    start_time = MPI_Wtime(); // Timer

    // MapReduce Entity
    MapReduce *mr = new MapReduce(MPI_COMM_WORLD);

    while(1) {  // PAGERANK LOOP

        // MAP
        mr->map(calculate, &data);
        MPI_Barrier(MPI_COMM_WORLD);

        /*
         * NOTE: The conventional method is to use collate(int) in this place, 
         * which is same as calling aggregate(int) followed by convert.
         * 
         * But, due to associativity & commutativity of addition of individual ranks, only convert() is enough to compute the pagerank,
         * still complying with the core idea of MapReduce!
         * 
         * Therefore, instead of ...
         *              mr->collate(graph_size+1); // Which is also valid, & working (tested)
         * ... we can use
         *              mr->convert(); // Only simplistic difference
         */
        
        // CONVERT
        mr->convert();
        MPI_Barrier(MPI_COMM_WORLD);

        // REDUCE
        mr->reduce(collect, new_rank);
        MPI_Barrier(MPI_COMM_WORLD);

        // COLLECTING RESULT
        MPI_Allreduce(MPI_IN_PLACE, new_rank, graph_size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        // Breakcheck
        if(difference(new_rank, rank) < LIMIT) break;

        // RESET
        for(int i=0; i<graph_size; i++) {
            rank[i] = new_rank[i];
            new_rank[i] = 0;
        }
        mr->reset();
    }

    // Logging
    if(!self && *argv[2]=='1') cerr << "Map Reduce Complete. Time " << (MPI_Wtime()-start_time) << "s.\n";

    // Writing Output
    if(!self && *argv[2]=='1') {
        for(int i=0; i<graph_size; i++)
            cout << i << " : " << setprecision(10) <<  rank[i] << "\n";
    }

    MPI_Finalize();
}
