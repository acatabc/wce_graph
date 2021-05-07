
#ifndef ALGENG_WCE_SOLVER_SOLVER_H
#define ALGENG_WCE_SOLVER_SOLVER_H

#include "WCE_Graph.h"
#include "../include/utils.h"
class Solver {
private:
    WCE_Graph *g;
    unsigned int rec_steps = 0;

public:
    Solver(WCE_Graph *);
    virtual ~Solver();

    void solve();
    int branch(int k, int layer);
    int branchEdge(int u, int v, int k, int layer);
    int branchEdgeAllowLoop(int u, int v, int k, int layer);

    std::tuple<int, int, int> find_next_p3();
    std::tuple<int, int, int> find_first_p3();
    std::tuple<int, int, int> get_max_cost_p3();

    void print_all_p3();
    std::list<std::tuple<int, int, int>> get_all_p3();

    std::tuple<int, int, int> get_max_cost_p3_experimental(int *p3_weight);
};


#endif //ALGENG_WCE_SOLVER_SOLVER_H
