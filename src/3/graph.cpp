
#include "graph.hpp"


Graph::Graph() {
    size_ = 0;
    link_count_ = 0;
    read_switch = false;
}


Graph::Graph(string filename) {
    size_ = 0;
    link_count_ = 0;
    read_switch = false;
    read_graph(filename);
}


void Graph::add_link(int _1, int _2) {
    if(outlink_count.find(_1) == outlink_count.end())
        outlink_count[_1] = 0;
    
    if(outlink_count.find(_2) == outlink_count.end())
        outlink_count[_2] = 0;
    
    if (_1 > size_) size_ = _1;
    if (_2 > size_) size_ = _2;
    
    outlink_count[_1] += 1;
    links_.insert({_1, _2});
    link_count_ += 1;
}


void Graph::read_graph(string filename) {
    assert(!read_switch);
    istringstream split;
    string _line; int _1, _2;
    
    try {
        ifstream _file = ifstream(filename);
    
        while(getline(_file, _line)) {
            split = istringstream(_line);
            split >> _1 >> _2;
            add_link(_1, _2);
        }
    
        read_switch = true;
        size_ += 1;     // Assuming 0 indexing
    }
    
    catch(...) {
        cerr << "Couldn't read file (" << filename << "). Abort.\n";
        exit(2);
    }
}


set<pair<int, int>>::iterator Graph::pair_iterator(bool end) {
    assert(read_switch);
    if (end) return links_.end();
    return links_.begin();
}


int Graph::total_outlinks(int index) {
    assert(read_switch);
    assert(index < size_);
    return outlink_count[index];
}


int Graph::size() {
    assert(read_switch);
    return size_;
}


int Graph::link_count() {
    assert(read_switch);
    return link_count_;
}