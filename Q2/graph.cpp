
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

void Graph::convert(graph_item** graph_list, int* g_size, int* d_size) {
    vector<graph_item> graph;

    double _s = 1/(double)size_;

    for(auto it=links_.begin(); it!=links_.end(); it++) {
        graph.push_back({1.0/((double)outlink_count[it->first]), {it->first, it->second}});
    }

    for(int i=0; i<size_; i++) {
        graph.push_back({_s, {i, -1}});
        if(outlink_count[i] == 0)
            graph.push_back({_s, {i, -2}});
    }

    shuffle(graph.begin(), graph.end(), default_random_engine());

    (*graph_list) = new graph_item[graph.size()];

    for(int i=0; i<graph.size(); i++)
        (*graph_list)[i] = graph[i];
    
    *g_size = size_;
    *d_size = graph.size();

    graph.clear();
}