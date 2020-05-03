
#include "mapreduce.hpp"

#include "graph.hpp"


#define DATATYPE pair<double *, graph_item *>


double ALPHA = 0.85;
int graph_size = 0, list_size = 0;
double LIMIT = 1e-13;

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



void calculate(int rank, int w_size, vector<kv_t> *kv, void *ptr) {
    double *rank_ = ((DATATYPE *)ptr)->first;
    graph_item *graph_ = ((DATATYPE *)ptr)->second;
    double v;
    int a, b;
    kv_t temp;

    int _start = (rank*list_size)/w_size, _end = ((rank + 1)*list_size)/w_size;

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
        
        v = (b > graph_size)?(v * (1-ALPHA)):(v * ALPHA);

        temp.key = b;
        temp.value = v;

        kv->push_back(temp);
    }
}



void collect(int key, vector<double>& values, void *ptr) {
    
    double total = 0;
    for(int i=0; i<values.size(); i++)
        total += values[i];
    
    double *data_ = (double *)ptr;
    
    if(key >= graph_size) {
        for(int i=0; i<graph_size; i++)
            data_[i] += total;
    }
    else
        data_[key] += total;
}



int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    int self, num_proc;

    MPI_Comm_rank(MPI_COMM_WORLD,&self);
    MPI_Comm_size(MPI_COMM_WORLD,&num_proc);

    graph_item* graph;
    Graph(argv[1]).convert(&graph, &graph_size, &list_size);

    double *rank = new double[graph_size](); rank[0] = 1;
    double *new_rank = new double[graph_size]();

    DATATYPE data = {rank, graph};

    MapReduce *mr = new MapReduce(MPI_COMM_WORLD);

    while(1) {

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
        
        mr->convert();

        MPI_Barrier(MPI_COMM_WORLD);

        mr->reduce(collect, new_rank);

        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Allreduce(MPI_IN_PLACE, new_rank, graph_size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);

        if(difference(new_rank, rank) < LIMIT) {
            if(!self) cerr << "BREAKPOINT!" << endl;
            break;
        }

        for(int i=0; i<graph_size; i++) {
            rank[i] = new_rank[i];
            new_rank[i] = 0;
        }

        mr->reset();
    }

    if(!self) {
        double sum = 0;
        for(int i=0; i<graph_size; i++) {
            sum += rank[i];
            cout << i << " : " << rank[i] << endl;
        }
        cerr << sum << endl;
        cerr << "DIFF: " << difference(new_rank, rank) << endl;
    }

    MPI_Finalize();
}
