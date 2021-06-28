#include "DeepSolver.h"

#include "iostream"
#include "../include/utils.h"
#include "HeuristicSolver.h"


DeepSolver::DeepSolver(WCE_Graph *g) : AbstractSolver(g){
}


void DeepSolver::solve() {

    // TODO data reduction before upper bound

    int heuristic_k = 500; //  get_upper_bound(); // TODO improvem upper bound, define search time
    upperBound =  heuristic_k;

    int c = branch(0,0);


    if(c == heuristic_k){  // best solution is the one found by the heuristic
        printDebug("Output heuristic solution");
//        output_heuristic_solution();
    }
    else{
        output_from_best_solution_stack();
        g->verify_cluster_graph();
    }

    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(c) + "\n");
}


int DeepSolver::branch(int c, int layer){

    // data reduction
    int stack_size_0 = g->graph_mod_stack.size(); // save stack size to recover current graph after data reduction
    int cost = data_reduction(upperBound - c, layer);
    if(cost == -1){ // data reduction shows that no solution for this k exists
        g->recover_graph(stack_size_0);
        printDebug("=== fail layer " + std::to_string(layer) + " (data red)");
        return upperBound;
    }
    else c += cost;

    if(c + get_lower_bound() >= upperBound) {
        g->recover_graph(stack_size_0);
        printDebug("=== fail layer " + std::to_string(layer) + " (upper bound)");
        return upperBound;
    }

    auto p3 = get_max_cost_p3(); // TODO mincost p3

    if(p3.i == -1){
        printDebug("FOUND CLUSTER GRAPH");
        save_into_best_solution_stack(g->graph_mod_stack);
        g->recover_graph(stack_size_0);
        return c;
    }

    rec_steps++;

    int u = p3.i; // it holds w(u,v) >= 0
    int v = p3.j;
    int w = p3.k;

    printDebug("Layer " + std::to_string(layer) + " P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");

    // TODO choose which edge of p3 to branch on
    // TODO choose first Merge vs Delete depending on which one is cheaper

    // 1. Branch DELETE (u,v)
    printDebug("Branch DELETE (" + std::to_string(u) + "," + std::to_string(v) + ")");
    int weight_uv = g->get_weight(u,v);
    int stack_size_1 = g->graph_mod_stack.size();
    g->set_non_edge(u, v);
    upperBound = this->branch(c + weight_uv, layer + 1);
    g->recover_graph(stack_size_1);

    // 2. Branch MERGE (u,v)
    printDebug("Branch MERGE (" + std::to_string(u) + "," + std::to_string(v) + ") -> " + std::to_string(g->merge_map.size()) );
    int merge_cost = g->merge(u,v);
    if(merge_cost == -1) {  // both (delete/merge) failed -> no solution for this k exists
        g->recover_graph(stack_size_0);
        printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");
        return upperBound;
    }
    upperBound = this->branch(c + merge_cost, layer + 1); // do not add weight(u,v) since (u,v) already exists
    g->recover_graph(stack_size_0);

    return upperBound;
}


int DeepSolver::get_lower_bound(){
//    std::tuple<Solver::p3, int> tuple = get_best_p3_and_lower_bound(MAX_SUM_P3, LOWER_BOUND_FAST);
//    int lowerBound = std::get<1>(tuple);
    return 0;
}



int DeepSolver::get_upper_bound(){

    HeuristicSolver h = HeuristicSolver(g);
    return h.upper_bound();
}



void DeepSolver::save_into_best_solution_stack(std::stack<WCE_Graph::stack_elem> current_stack){
    while(!best_solution_stack.empty()){
        best_solution_stack.pop();
    }
    while(!current_stack.empty()){
        best_solution_stack.push(current_stack.top());
        current_stack.pop();
    }
}


// outputs edges in "best_solution_heuristic"
void DeepSolver::output_from_best_solution_stack(){
    while(!best_solution_stack.empty()){
        WCE_Graph::stack_elem el = best_solution_stack.top();
        if(el.type == MERGE) {
//            if(g->get_weight_original(el.v1, el.v2) <= 0){
//                final_output(el.v1, el.v2);
//            }
            g->merge(el.v1, el.v2);
        }
        else if(el.type == SET_INF) {
//            if(g->get_weight_original(el.v1, el.v2) > 0){
//                final_output(el.v1, el.v2);
//            }
            g->set_non_edge(el.v1, el.v2);
        }
        else if(el.type == CLIQUE){
            g->remove_clique(el.clique);
        }
        best_solution_stack.pop();

    }
    g->verify_cluster_graph();
    clear_stack_and_output();
}


int DeepSolver::data_reduction(int k, int layer){
    int k_before = k;

//    if(layer %5 ==  0 && layer >= 10){
////      this->dataRed_heavy_non_edge();
////      k = dataRed_heavy_edge_single_end(k);
//      k = dataRed_heavy_non_edge(k);
//      k = dataRed_heavy_edge_single_end(k);
//      k = dataRed_large_neighbourhood_I(k);
//      k = dataRed_heavy_edge_both_ends(k);
//    }

    k = dataRed_weight_larger_k(k);
    if(k == -1) return -1;

//    if(k != k_before)
//        printDebug("Data reduction reduced k to " + std::to_string(k));
    return k_before - k;
}



// iterates over all vertex tuples and returns max_cost p3
DeepSolver::p3 DeepSolver::get_max_cost_p3(){

    int u = -1;
    int v = -1;
    int w = -1;
    int max_cost = INT32_MIN;
    for(int i: this->g->active_nodes){
        for(int j: this->g->active_nodes){
            for(int k : this->g->active_nodes){
                if(i == j || i == k || k == j) continue;
                int weight_i_j = g->get_weight(i,j);
                int weight_i_k = g->get_weight(i,k);
                int weight_j_k = g->get_weight(j,k);
                if(weight_i_j > 0 && weight_i_k > 0 && weight_j_k <= 0){

                    // sum up costs of all three edges (only edges that are allowed to be modified)
                    int current_cost = 0;
                    if(weight_i_k != DO_NOT_DELETE && weight_i_k != DO_NOT_ADD) current_cost += abs(weight_i_k);
                    if(weight_i_j != DO_NOT_DELETE && weight_i_j != DO_NOT_ADD) current_cost += abs(weight_i_j);
                    if(weight_j_k != DO_NOT_DELETE && weight_j_k != DO_NOT_ADD) current_cost += abs(weight_j_k);

                    // update maximum cost and corresponding p3
                    if(current_cost > max_cost) {
                        max_cost = current_cost;
                        u = i;
                        v = j;
                        w = k;
                    }
                }
            }

        }
    }
    return DeepSolver::p3{.i = u, .j = v, .k = w, .cost_sum = -1 ,.min_cost = -1};
}

