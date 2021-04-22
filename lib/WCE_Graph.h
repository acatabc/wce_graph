//
// Created by acat on 16.04.21.
//

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
    int num_vertices;
    void add_edge(int,int);
    void delete_edge(int, int);
    void modify_edge(int, int);
    void set_weight(int , int , int );
    int get_weight(int,int);
    virtual ~WCE_Graph();

    void print(std::ostream &os);
};


#endif //ALGENG_WCE_SOLVER_WCE_GRAPH_H
