

#include "AbstractSolver.h"

#ifndef ALGENG_WCE_SOLVER_DEEPSOLVER_H
#define ALGENG_WCE_SOLVER_DEEPSOLVER_H

#endif //ALGENG_WCE_SOLVER_DEEPSOLVER_H


class DeepSolver : AbstractSolver {
    int upperBound = 2000;
public:
    DeepSolver(WCE_Graph *);

    void solve();
    int branch(int k, int layer);


    p3 get_max_cost_p3();

    int get_lower_bound();
    int get_upper_bound();

    std::stack<WCE_Graph::stack_elem> best_solution_stack = std::stack<WCE_Graph::stack_elem>();
    void save_current_solution(std::stack<WCE_Graph::stack_elem> current_stack);
    void output_best_solution();

};
