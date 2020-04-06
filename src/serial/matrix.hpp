

#include "graph.hpp"


// Sparse Matrix Class
class Sparse_Matrix {
private:

    int order_;  // Order of one dimension of the matrix
    unordered_set<int> dangling;    // Dangling node indices
    map<int, unordered_set<int>> row;         // non-zero values in a row, all indices
    int *column_values;     // non-zero entries count at each column, values = 1/coloumn_values

public:


    /*
     * Sparse Matrix (Constructor)
     * @param (Graph&): the graph used for constructing the matrix
     */
    Sparse_Matrix(Graph&);


    /*
     * Vector dot-product step
     * @param (vector<double>&): the step-vector, for product
     * @param (double): the alpha value in the algorithm
     */
    vector<double> step(vector<double>&, double);

    /*
     * Order of the matrix (square-matrix)
     */ 
    int order();
};