


#include "matrix.hpp"


/*
 * Euclidean difference b/w vectors
 * @param (_1, _2): vectors
 */
double difference(vector<double> _1, vector<double> _2) {
    double diff = 0;
    for(size_t i{}; i<_1.size(); i++)
        diff += pow(_1[i] - _2[i], 2);
    return diff;
}



/*
 * The final pagerank algorithm.
 * @param (string): local path of the graph file
 * 
 * @return: the pagerank of all involved nodes
 */
vector<double> pagerank(string filename, double limit = 1e-13, double alpha = 0.85) {
    // Reading the graph
    double start_time = MPI_Wtime();
    Graph g = Graph(filename);
    cerr << "Graph Read. Time " << (MPI_Wtime()-start_time) << "s.\n";
    // Constructing the matrix
    start_time = 0;
    Sparse_Matrix matrix = Sparse_Matrix(g);
    cerr << "Matrix Made. Time " << (MPI_Wtime()-start_time) << "s.\n";
    // Step with the matrix, until convergence
    vector<double> _1, _2;  //vectors
    _1 = _2 = vector<double>(matrix.order());
    _1[0] = 1;  // starting vector
    double _df; bool _s;    // local variables
    start_time = 0;
    do {    // loop
        _2 = _1;
        _1 = matrix.step(_1, alpha);
        // Calculating the difference
        _df = difference(_1, _2);
    } while(_df > limit);
    cerr << "Complete. Time " << (MPI_Wtime()-start_time) << "s.\n";
    // Return the value
    return _1;
}


int main(int argc, char const *argv[])
{
    /* code */
    vector<double> answer = pagerank(argv[1]);
    return 0;
}
