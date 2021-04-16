//
// Created by acat on 16.04.21.
//

#ifndef ALGENG_WCE_SOLVER_WCE_GRAPH_H
#define ALGENG_WCE_SOLVER_WCE_GRAPH_H


class WCE_Graph {
private:
    int **adj_matrix;
public:
    WCE_Graph(int n);
    void add_edge();
    void delete_edge();
    void find_p3();
    void solve();
};


#endif //ALGENG_WCE_SOLVER_WCE_GRAPH_H
