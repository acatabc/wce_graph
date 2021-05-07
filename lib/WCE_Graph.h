
#ifndef ALGENG_WCE_SOLVER_WCE_GRAPH_H
#define ALGENG_WCE_SOLVER_WCE_GRAPH_H
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>
#include <list>


class WCE_Graph {
private:
    int **adj_matrix;

public:
    WCE_Graph(int);
    int num_vertices_original;

    void add_edge(int,int);
    void delete_edge(int, int);
    void modify_edge(int, int);
    void set_weight(int , int , int );
    int get_weight(int,int);
    int get_p3_cost(int u, int v, int w);
    virtual ~WCE_Graph();
    int get_cost(int u, int v, int w);
    void printGraph(std::ostream &os);
};

#endif //ALGENG_WCE_SOLVER_WCE_GRAPH_H
