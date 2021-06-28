
#ifndef ALGENG_WCE_SOLVER_SOLVER_H
#define ALGENG_WCE_SOLVER_SOLVER_H

#include <vector>
#include <map>
#include "WCE_Graph.h"
#include "AbstractSolver.h"

#define MAX_SUM_P3 0
#define MAX_MIN_EDGE_P3 1
#define LOWER_BOUND_FAST 0
#define LOWER_BOUND_IMPROVED 1

#define MAX_NUM_VERTICES 1000


class Solver : public AbstractSolver {
public:
    Solver();

    void solve();
    int branch(int k, int layer);


    p3 get_max_cost_p3();
    std::tuple<p3, int> get_best_p3_and_lower_bound(int heuristic, int version);
    std::vector<p3> find_all_p3();

    int data_reduction(int k, int layer);
};


#endif //ALGENG_WCE_SOLVER_SOLVER_H
