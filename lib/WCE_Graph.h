//
// Created by acat on 16.04.21.
//

#ifndef ALGENG_WCE_SOLVER_WCE_GRAPH_H
#define ALGENG_WCE_SOLVER_WCE_GRAPH_H
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>


class WCE_Graph {
private:
    int **adj_matrix;
    int num_vertices;

    //helper variables
    int p3_i, p3_j = 0;
public:
    WCE_Graph(int);
    void add_edge(int,int,int);
    void delete_edge(int, int);
    int branch(int k );
    std::tuple<int, int, int> find_next_p3();
    std::tuple<int, int, int> find_first_p3();
    void solve();
    virtual ~WCE_Graph();

    void print(std::ostream &os);
};


#endif //ALGENG_WCE_SOLVER_WCE_GRAPH_H
