


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
vector<double> pagerank(string filename, double limit = 1e-7, double alpha = 0.85) {
    // Reading the graph
    cerr <<  "Reading Graph." << endl;
    Graph g = Graph(filename);
    fprintf(__stderrp, "Graph Read.\nSize: %d.\nLinks: %d.\n", g.size(), g.link_count());
    // Constructing the matrix
    cerr <<  "Constructing Table." << endl;
    Sparse_Matrix matrix = Sparse_Matrix(g);
    cerr <<  "Table Complete." << endl;
    // Step with the matrix, until convergence
    vector<double> _1, _2;  //vectors
    _1 = _2 = vector<double>(matrix.order());
    _1[0] = 1;  // starting vector
    double _df; bool _s;    // local variables
    do {    // loop
        _2 = _1;
        _1 = matrix.step(_1, alpha);
        // Calculating the difference
        _df = difference(_1, _2);
        fprintf(__stderrp, "Difference %lf\n", _df);
    } while(_df > limit);
    // Return the value
    return _1;
}


int main(int argc, char const *argv[])
{
    /* code */
    vector<double> answer = pagerank(argv[1]);
    for(size_t i{}; i<answer.size(); i++)
        cout << i << " = " << answer[i] << endl;
    return 0;
}
