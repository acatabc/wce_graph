#include "DeepSolver.h"

#include "iostream"
#include "../include/utils.h"
#include "HeuristicSolver.h"


DeepSolver::DeepSolver(WCE_Graph *g) : AbstractSolver(g){}

void DeepSolver::solve() {

    int dataRed_cost = data_reduction_before_branching();

    int k_heuristic =  get_upper_bound() + dataRed_cost; // TODO improvem upper bound, define search time

    upperBound = k_heuristic;

    int k = branch(dataRed_cost,0);
    g->recover_graph(0);

    output_best_solution();
    g->verify_cluster_graph();

    std::cout << "#recursive steps: " << rec_steps << std::endl;
    std::cout << "#heuristic k:     " << k_heuristic << std::endl;
    std::cout << "#final k:         " << k << std::endl;
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

    if(c + get_lower_bound() > upperBound || (c + get_lower_bound() == upperBound && !best_solution_stack.empty())) {
        g->recover_graph(stack_size_0);
        printDebug("=== fail layer " + std::to_string(layer) + " (upper bound)");
        return upperBound;
    }

    auto p3 = get_min_edge_p3();

    if(p3.i == -1){
        printDebug("FOUND CLUSTER GRAPH");
        save_current_solution(g->graph_mod_stack);
        g->recover_graph(stack_size_0);
        return c;
    }

    rec_steps++;

    printDebug("Layer " + std::to_string(layer) + " P3 (" + std::to_string(p3.i) + "," + std::to_string(p3.j) + ","+ std::to_string(p3.k) + ")");

    // choose first Merge vs Delete depending on which one is cheaper
    // get min cost edge uv
    // a) uv == ij --> 1) delete ij 2) merge ij
    // b) uv != ij --> 1) merge ij  2) delete ij
    if(get_idx_min_edge(p3) == 0) upperBound = branch_edge_first_delete(c, p3.i, p3.j, layer);
    else upperBound = branch_edge_first_merge(c, p3.i, p3.j, layer);

    g->recover_graph(stack_size_0);

    return upperBound;
}



int DeepSolver::get_idx_min_edge(DeepSolver::p3 &p3){
    std::vector<int> weights = std::vector<int>();
    weights.push_back(g->get_weight(p3.i, p3.j));
    weights.push_back(g->get_weight(p3.i, p3.k));
    weights.push_back(g->get_weight(p3.j, p3.k));
    int min_idx = -1;
    int min = INT32_MAX;
    int i = 0;
    for(int w : weights){
        if(w != DO_NOT_ADD && abs(w) < min){
            min_idx = i;
            min = abs(w);
        }
        i++;
    }
    return min_idx;
}

int DeepSolver::branch_edge_first_delete(int c, int u, int  v, int layer){
    // first delete, then merge
    int stack_size_1 = g->graph_mod_stack.size();

    // 1. Branch DELETE (u,v)
    printDebug("Branch DELETE (" + std::to_string(u) + "," + std::to_string(v) + ")");
    int weight_uv = g->get_weight(u,v);
    g->set_non_edge(u, v);
    upperBound = this->branch(c + weight_uv, layer + 1);
    g->recover_graph(stack_size_1);

    // 2. Branch MERGE (u,v)
    printDebug("Branch MERGE (" + std::to_string(u) + "," + std::to_string(v) + ") -> " + std::to_string(g->merge_map.size()) );
    int merge_cost = g->merge(u,v);
    if(merge_cost == -1) {  // both (delete/merge) failed -> no solution for this k exists
        g->recover_graph(stack_size_1);
        printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) +  ")");
        return upperBound;
    }
    upperBound = this->branch(c + merge_cost, layer + 1); // do not add weight(u,v) since (u,v) already exists
    g->recover_graph(stack_size_1);
    return upperBound;
}


int DeepSolver::branch_edge_first_merge(int c, int u, int  v, int layer){
    // first merge, then delete
    int stack_size_1 = g->graph_mod_stack.size();

    // 1. Branch MERGE (u,v)
    printDebug("Branch MERGE (" + std::to_string(u) + "," + std::to_string(v) + ") -> " + std::to_string(g->merge_map.size()) );
    int merge_cost = g->merge(u,v);
    if(merge_cost == -1) {  // both (delete/merge) failed -> no solution for this k exists
        g->recover_graph(stack_size_1);
        printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) +  ")");
        return upperBound;
    }
    upperBound = this->branch(c + merge_cost, layer + 1); // do not add weight(u,v) since (u,v) already exists
    g->recover_graph(stack_size_1);

    // 1. Branch DELETE (u,v)
    printDebug("Branch DELETE (" + std::to_string(u) + "," + std::to_string(v) + ")");
    int weight_uv = g->get_weight(u,v);
    g->set_non_edge(u, v);
    upperBound = this->branch(c + weight_uv, layer + 1);
    g->recover_graph(stack_size_1);
    return upperBound;
}


int DeepSolver::get_lower_bound(){
//    std::tuple<Solver::p3, int> tuple = get_best_p3_and_lower_bound(MAX_SUM_P3, LOWER_BOUND_FAST);
//    int lowerBound = std::get<1>(tuple);
    return 0;
}



int DeepSolver::get_upper_bound(){
    HeuristicSolver h = HeuristicSolver(g);
    return h.compute_upper_bound();
}



void DeepSolver::save_current_solution(std::stack<WCE_Graph::stack_elem> current_stack){
    while(!best_solution_stack.empty()){
        best_solution_stack.pop();
    }
    while(!current_stack.empty()){
        best_solution_stack.push(current_stack.top());
        current_stack.pop();
    }
}


// outputs edges in "best_solution_heuristic"
void DeepSolver::output_best_solution(){
    // recover graph from best_solution_stack
    while(!best_solution_stack.empty()){
        WCE_Graph::stack_elem el = best_solution_stack.top();
        if(el.type == MERGE) {
            g->merge(el.v1, el.v2);
        }
        else if(el.type == SET_INF) {
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


// iterates over all vertex tuples and returns min_min_edge p3
DeepSolver::p3 DeepSolver::get_min_cost_p3(){

    DeepSolver::p3 best_p3 = {.i = -1, .j = -1, .k = -1, .cost_sum = -1 ,.min_cost = -1};
    int min_cost = INT32_MAX;
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

                    // update minimum cost and corresponding p3
                    if(current_cost < min_cost) {
                        min_cost = current_cost;
                        best_p3.i = i;
                        best_p3.j = j;
                        best_p3.k = k;
                        best_p3.cost_sum = current_cost;
                    }
                }
            }

        }
    }
    return best_p3;
}


// iterates over all vertex tuples and returns min_min_edge p3
DeepSolver::p3 DeepSolver::get_min_edge_p3(){

    DeepSolver::p3 best_p3 = {.i = -1, .j = -1, .k = -1, .cost_sum = -1 ,.min_cost = -1};
    int min_cost = INT32_MAX;
    for(int i: this->g->active_nodes){
        for(int j: this->g->active_nodes){
            for(int k : this->g->active_nodes){
                if(i == j || i == k || k == j) continue;
                int weight_i_j = g->get_weight(i,j);
                int weight_i_k = g->get_weight(i,k);
                int weight_j_k = g->get_weight(j,k);
                if(weight_i_j > 0 && weight_i_k > 0 && weight_j_k <= 0){

                    // consider edges for update min_cost (only edges that are allowed to be modified)
                    bool improved = false;
                    if(!improved && weight_i_k != DO_NOT_DELETE && weight_i_k != DO_NOT_ADD && abs(weight_i_k) < min_cost) {
                        min_cost = abs(weight_i_k);
                        improved = true;
                    }
                    if(!improved && weight_i_j != DO_NOT_DELETE && weight_i_j != DO_NOT_ADD && abs(weight_i_j) < min_cost) {
                        min_cost = abs(weight_i_j);
                        improved = true;
                    }
                    if(!improved && weight_j_k != DO_NOT_DELETE && weight_j_k != DO_NOT_ADD && abs(weight_j_k) < min_cost) {
                        min_cost = abs(weight_j_k);
                        improved = true;
                    }

                    // update minimum cost and corresponding p3
                    if(improved) {
                        best_p3.i = i;
                        best_p3.j = j;
                        best_p3.k = k;
                        best_p3.min_cost = min_cost;
                    }
                }
            }

        }
    }
    return best_p3;
}

