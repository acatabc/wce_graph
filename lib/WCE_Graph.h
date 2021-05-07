
#ifndef ALGENG_WCE_SOLVER_WCE_GRAPH_H
#define ALGENG_WCE_SOLVER_WCE_GRAPH_H
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>
#include <list>
#include <vector>


class WCE_Graph {
private:
    struct matrix_entry{
        int weight;
        bool flag;
    };
    std::vector<std::vector<matrix_entry>> adj_matrix;

public:
    WCE_Graph(int);
    const int num_vertices;
    virtual ~WCE_Graph();

    void set_weight(int, int, int);
    int get_weight(int, int);

    void add_edge(int,int);
    void delete_edge(int, int);
    void modify_edge(int, int); // old, just used in solver1()

    int get_cost(int u, int v, int w);

    // merging
    std::vector<int> active_nodes;
    std::vector<std::vector<int>> merge_map;
    int merge(int, int);
    void unmerge(int);
    void recover_original();

    void printGraph(std::ostream &os);
    void print_active_graph(std::ostream &os);
    void print_active_nodes();
    void print_merge_map();
};

#endif //ALGENG_WCE_SOLVER_WCE_GRAPH_H
