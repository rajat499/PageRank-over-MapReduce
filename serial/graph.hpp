// Graph Library


#include "include.hpp"


class Graph {
private:

    int size_;      // size of the graph
    int link_count_;    // total number of links
    bool read_switch;     // checking whether the graph has been read
    unordered_map<int, int> outlink_count;     // count of links going out from a node
    set<pair<int, int>> links_;     // out-link count from each node


    /*
     * Adding a new link in the graph
     * @param (_1, _2): _1 -> _2, link parameters
     */
    void add_link(int, int);

public:

    /*
     * Empty graph constructor
     */
    Graph();


    /*
     * Graph constructor, with filename
     * @param (string): local filepath, for the graph file
     */
    Graph(string);


    /*
     * Reading the complete graph from a file
     * @param filename (string): local filepath for the graph file
     */
    void read_graph(string filename);


    /*
     * Get iterator for node list
     * @param end (bool): end or starting iterator
     */
    set<pair<int, int>>::iterator pair_iterator(bool end);


    /*
     * Total outlinks from a particular node
     * @param index: index of the considered node
     */
    int total_outlinks(int index);


    /*
     * Size of the graph
     */
    int size();

    /*
     * Total number of links in the graph
     */
    int link_count();
};