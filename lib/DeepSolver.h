

#include "AbstractSolver.h"

#ifndef ALGENG_WCE_SOLVER_DEEPSOLVER_H
#define ALGENG_WCE_SOLVER_DEEPSOLVER_H

#endif //ALGENG_WCE_SOLVER_DEEPSOLVER_H


class DeepSolver : AbstractSolver {
    int upperBound = INT32_MAX;
public:
    DeepSolver(WCE_Graph *);

    void solve();
    int branch(int k, int layer);

    int branch_edge_first_delete(int k, int u, int v, int layer);
    int branch_edge_first_merge(int k, int u, int v, int layer);

    p3 get_min_cost_p3();
    p3 get_min_edge_p3();
    int get_idx_min_edge(DeepSolver::p3 &p3);

    int get_lower_bound();
    int get_lower_bound_cplex();
    int get_upper_bound();

    // output
    std::stack<WCE_Graph::stack_elem> best_solution_stack = std::stack<WCE_Graph::stack_elem>();
    void save_current_solution(std::stack<WCE_Graph::stack_elem> current_stack);
    void output_best_solution();
};
