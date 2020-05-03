

#include "matrix.hpp"


Sparse_Matrix::Sparse_Matrix(Graph& g) {
    // Setting the order
    order_ = g.size();     // assuming g is 0 indexed
    
    // Local Variables
    int _1, _2;
    
    // Compression Quality
    cerr << "Percentage Compression: " << (100 * (double)g.link_count() / pow((double)g.size(), 2)) << "%\n";
    
    // Switch for checking set creation
    bool* set_ = new bool[order_]();
    
    // Setting column values
    column_values = new int[order_]();
    
    // Adding nodes to the respective values
    for(auto it = g.pair_iterator(0); it != g.pair_iterator(1); it++) {
        _1 = it->first, _2 = it->second;
        if(!set_[_2]) {
            set_[_2] = true;
            row[_2] = unordered_set<int>();
        }
        row[_2].insert(_1);
    }
    
    // Setting the danling nodes, and column values
    for(size_t i{}; i < order_; i++) {
        _1 = g.total_outlinks(i);
        if(!_1)
            dangling.insert(i);
        else
            column_values[i] = _1;
    }
}


vector<double> Sparse_Matrix::step(vector<double>& rank, double alpha) {
    vector<double> new_rank = vector<double>(order_, 0);
    
    // calculating _alpha * A * I, constant over all rows
    double dangling_value = 0;
    for(int index: dangling)
        dangling_value += rank[index];
    dangling_value /= (double)order_;
    
    // Calculating (1 - _alpha) * [1] / N, constal over all rows
    double random_value = 0;
    for(size_t i{}; i < order_; i++)
        random_value += rank[i];
    random_value /= (double)order_;
    
    // Finally multiplying with the sparse matrix
    for(size_t i{}; i < order_; i++) {
        double value = 0;
        for(int index: row[i])
            value += (rank[index] / (double)column_values[index]);
        new_rank[i] = alpha * (value + dangling_value) + (1 - alpha) * (random_value);
    }
    
    // Return the new calculated vector
    return new_rank;
}


int Sparse_Matrix::order() {
    return order_;
}