
#include "Solver.h"
#include <tuple>
#include "iostream"
#include "../include/utils.h"

Solver::Solver(WCE_Graph *g) : AbstractSolver(g){}

void Solver::solve() {

    int cost_before_branching = data_reduction_before_branching();
    int stack_before_branching = g->graph_mod_stack.size();

    // compute lower bound iterating over all existent p3s
    int lower_bound_k = std::get<1>(get_best_p3_and_lower_bound(MAX_SUM_P3, LOWER_BOUND_FAST));

    int k = lower_bound_k + cost_before_branching;

    while (true){
        printDebug("\nSOLVE FOR k:" + std::to_string(k));

        int k_reduced = k - cost_before_branching;

        if(this->branch(k_reduced, 0) == CLUSTER_GRAPH){
            clear_stack_and_output();
            break;
        }
        g->recover_graph(stack_before_branching);
        k++;
    }

    g->verify_cluster_graph();

    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(k) + "\n");
}


int Solver::branch(int k, int layer){
    if(k < 0){
        return NONE;
    }

    // data reduction
    int stack_size_0 = g->graph_mod_stack.size(); // save stack size to recover current graph after data reduction
    int cost = data_reduction(k, layer);
    if(cost == -1){ // data reduction shows that no solution for this k exists
        g->recover_graph(stack_size_0);
        printDebug("=== fail layer " + std::to_string(layer) + " (data red)");
        return NONE;
    }
    else k -= cost;

    // get best p3 and compute a lower bound
    std::tuple<Solver::p3, int> tuple;
    tuple = get_best_p3_and_lower_bound(MAX_SUM_P3, LOWER_BOUND_FAST);
    Solver::p3 p3 = std::get<0>(tuple);
    int lower_bound_k = std::get<1>(tuple);
    if(lower_bound_k > k) {
        printDebug("Successfully applied lower bound");
        return NONE;
    }

    if(p3.i == -1){
        printDebug("FOUND CLUSTER GRAPH");
        return CLUSTER_GRAPH;
    }

    rec_steps++;

    int u = p3.i;
    int v = p3.j;
    int w = p3.k;

    printDebug("Layer " + std::to_string(layer) + " P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");

    // 1. Branch DELETE (u,v)
    printDebug("Branch DELETE (" + std::to_string(u) + "," + std::to_string(v) + ")");
    int weight_uv = g->get_weight(u,v); // it holds (u,v) >= 0
    int stack_size_1 = g->graph_mod_stack.size();
    g->set_non_edge(u, v);
    if(this->branch(k - weight_uv, layer + 1) == CLUSTER_GRAPH){
        return CLUSTER_GRAPH;
    }
    else g->recover_graph(stack_size_1);

    // 2. Branch MERGE (u,v)
    // since deleting failed any solution must contain (u,v)
    // after merging (u,v) the p3 (u,v,w) will be resolved
    printDebug("Branch MERGE (" + std::to_string(u) + "," + std::to_string(v) + ") -> " + std::to_string(g->merge_map.size()) );
    cost = g->merge(u,v);
    if(cost == -1) {  // both (delete/merge) failed -> no solution for this k exists
        g->recover_graph(stack_size_0);
        printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");
        return NONE;
    }
    else k -= cost;
    if(this->branch(k, layer + 1) == CLUSTER_GRAPH){
        return CLUSTER_GRAPH;
    }

    // both branches (delete/merge) failed, no solution for this k exists, recover graph before branching
    g->recover_graph(stack_size_0);
    printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");
    return NONE;
}




