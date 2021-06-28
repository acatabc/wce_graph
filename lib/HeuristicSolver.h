//
// Created by Julia Henkel on 28.06.21.
//

#ifndef ALGENG_WCE_SOLVER_HEURISTICSOLVER_H
#define ALGENG_WCE_SOLVER_HEURISTICSOLVER_H
#include "WCE_Graph.h"

class HeuristicSolver {
    WCE_Graph *g;
public:
    HeuristicSolver(WCE_Graph *);


    // heuristics
    //bool terminate = false;
    void solve();
    void heuristic0();
    void heuristic1();
    void heuristic2();
    int upper_bound();
    static void signal_handler(int signal);

    void localSearch();

    void random_cluster_graph();
    int clusterMove(int u, int v);
    std::vector<int> compute_vertex_cost();

    int best_k = INT32_MAX;
    std::vector<std::pair<int,int>> best_solution;
    void save_best_solution();
    void output_heuristic_solution();
    void verify_best_solution();

    void output_modified_edges();
    int compute_modified_edge_cost();

};


#endif //ALGENG_WCE_SOLVER_HEURISTICSOLVER_H
