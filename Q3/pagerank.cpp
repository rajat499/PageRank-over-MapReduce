
#include "include/mapreduce.h"
#include "include/keyvalue.h"

#include "graph.hpp"


using namespace MAPREDUCE_NS;


#define DATATYPE pair<double *, graph_item *>


double ALPHA = 0.85;
int graph_size = 0, list_size = 0;
double LIMIT = 1e-13;

int self, num_proc;


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



void calculate(int task_num, KeyValue *kv, void *ptr) {
    double *rank_ = ((DATATYPE *)ptr)->first;
    graph_item *graph_ = ((DATATYPE *)ptr)->second;
    double v;
    int a, b;

    int _start = (self*list_size)/num_proc, _end = ((self + 1)*list_size)/num_proc;

    for(int i=_start; i<_end; i++) {
        v = graph_[i].first;
        a = graph_[i].second.first;
        b = graph_[i].second.second;

        v = rank_[a] * v;

        if(v == 0) continue;
        
        if(b == -2) // DANGLING
            b = graph_size;
        else if(b == -1) // RANDOM JUMP
            b = graph_size + 1;
        
        kv->add((char *)&b, sizeof(int), (char *)&v, sizeof(double));
    }
}



void collect(char *key, int keybytes, char *multivalue, int nvalues, int *valuebytes, KeyValue *kv, void *ptr) {
    int index = *(int *)key;
    
    double total = 0;
    for(int i=0; i<nvalues; i++) 
        total += (*(double *)(multivalue + sizeof(double) * i));
    
    double *data_ = (double *)ptr;
    
    if(index == graph_size+1)
        for(int i=0; i<graph_size; i++)
            data_[i] += total * (1 - ALPHA);
    else if(index == graph_size)
        for(int i=0; i<graph_size; i++)
            data_[i] += total * ALPHA;
    else
        data_[index] += total * ALPHA;
}



int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD,&self);
    MPI_Comm_size(MPI_COMM_WORLD,&num_proc);

    double start_time = MPI_Wtime();

    graph_item* graph;
    Graph(argv[1]).convert(&graph, &graph_size, &list_size);

    if(!self) 
        cerr << "Graph Read. Size " << graph_size << ". Link Size " << list_size << ". Time " << (MPI_Wtime()-start_time) << "s.\n";

    double *rank = new double[graph_size](); rank[0] = 1;
    double *new_rank = new double[graph_size]();

    DATATYPE data = {rank, graph};

    start_time = MPI_Wtime();

    while(1) {
        
        MapReduce *mr = new MapReduce(MPI_COMM_WORLD);

        mr->map(num_proc, calculate, &data);

        MPI_Barrier(MPI_COMM_WORLD);

        /*
         * NOTE: The conventional method is to use collate(int) in this place, 
         * which is same as calling aggregate(int) followed by convert.
         * 
         * But, due to associativity & commutativity of addition of individual ranks, only convert() is enough to compute the pagerank,
         * still complying with the core idea of MapReduce!
         * 
         * Therefore, instead of ...
         *              mr->collate(NULL); // Which is also valid, & working (tested)
         * ... we can use
         *              mr->convert(); // Only simplistic difference
         */

        int key_count = mr->convert();

        MPI_Barrier(MPI_COMM_WORLD);

        mr->reduce(collect, new_rank);

        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Allreduce(MPI_IN_PLACE, new_rank, graph_size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);

        if(difference(new_rank, rank) < LIMIT) {
            break;
        }

        for(int i=0; i<graph_size; i++) {
            rank[i] = new_rank[i];
            new_rank[i] = 0;
        }

        delete mr;
    }

    if(!self) cerr << "Map Reduce Complete. Time " << (MPI_Wtime()-start_time) << "s.\n";

    if(!self) {
        ofstream out_(argv[2]);

        for(int i=0; i<graph_size; i++)
            out_ << i << " : " << setprecision(10) <<  rank[i] << "\n";
        
        out_.close();
    }

    MPI_Finalize();
}
