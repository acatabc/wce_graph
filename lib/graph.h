//
// Created by acat on 16.04.21.
//

#ifndef ALGENG_WCE_SOLVER_GRAPH_H
#define ALGENG_WCE_SOLVER_GRAPH_H


class Graph {
private:
    int **adj_matrix;
public:
    Graph(int n);
    void add_vertex();
    void delete_vertex();
    void add_edge();
    void delete_edge();
};


#endif //ALGENG_WCE_SOLVER_GRAPH_H
