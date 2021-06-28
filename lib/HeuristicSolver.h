//
// Created by Julia Henkel on 28.06.21.
//

#ifndef ALGENG_WCE_SOLVER_HEURISTICSOLVER_H
#define ALGENG_WCE_SOLVER_HEURISTICSOLVER_H
#include "WCE_Graph.h"

class HeuristicSolver {
private:
    WCE_Graph *g;

    int greedy_cluster_graph();
    int move_to_cluster(int u, int v);
    int local_search();

    int best_k = INT32_MAX;
    std::vector<std::pair<int,int>> best_solution;

    void save_current_solution();
    void output_best_solution();
    void verify_best_solution();

    static void signal_handler(int signal);

public:
    HeuristicSolver(WCE_Graph *);

    void solve();
    int compute_upper_bound();

};


#endif //ALGENG_WCE_SOLVER_HEURISTICSOLVER_H
