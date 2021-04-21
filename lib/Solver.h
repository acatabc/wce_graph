//
// Created by Julia Henkel on 20.04.21.
//

#ifndef ALGENG_WCE_SOLVER_SOLVER_H
#define ALGENG_WCE_SOLVER_SOLVER_H

#include "WCE_Graph.h"

class Solver {
private:
    WCE_Graph *g;
public:
    Solver(WCE_Graph *);
    virtual ~Solver();

    void solve();
    int branch(int k);
    int branchEdge(int u, int v, int k);
    std::tuple<int, int, int> find_next_p3();
    std::tuple<int, int, int> find_first_p3();

    void print_all_p3();
    void printDebug(std::string);

    std::list<std::tuple<int, int, int>> get_all_p3();
    std::tuple<int, int, int> get_max_cost_p3(int *costs);

    std::tuple<int, int, int> get_max_cost_p3_experimental(int *p3_weight);
};


#endif //ALGENG_WCE_SOLVER_SOLVER_H
