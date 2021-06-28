#include "Solver.h"

#include "iostream"
#include "../include/utils.h"

void Solver::deepS() {

    // TODO data reduction before upper bound

    int heuristic_k = 500; //  get_upper_bound(); // TODO improvem upper bound, define search time
    upperBound =  heuristic_k;

    int c = deepB(0,0);


    if(c == heuristic_k){  // best solution is the one found by the heuristic
        printDebug("Output heuristic solution");
        output_heuristic_solution();
    }
    else{
        output_from_best_solution_stack();
        verify_cluster_graph();
    }

    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(c) + "\n");
}


int Solver::deepB(int c, int layer){

    // data reduction
    int stack_size_0 = g->graph_mod_stack.size(); // save stack size to recover current graph after data reduction
    int cost = deep_data_reduction(upperBound - c, layer);
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
    upperBound = this->deepB(c + weight_uv, layer + 1);
    g->recover_graph(stack_size_1);

    // 2. Branch MERGE (u,v)
    printDebug("Branch MERGE (" + std::to_string(u) + "," + std::to_string(v) + ") -> " + std::to_string(g->merge_map.size()) );
    int merge_cost = g->merge(u,v);
    if(merge_cost == -1) {  // both (delete/merge) failed -> no solution for this k exists
        g->recover_graph(stack_size_0);
        printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");
        return upperBound;
    }
    upperBound = this->deepB(c + merge_cost, layer + 1); // do not add weight(u,v) since (u,v) already exists
    g->recover_graph(stack_size_0);

    return upperBound;
}


int Solver::get_lower_bound(){
    std::tuple<Solver::p3, int> tuple = get_best_p3_and_lower_bound(MAX_SUM_P3, LOWER_BOUND_FAST);
    int lowerBound = std::get<1>(tuple);
    return lowerBound;
}



int Solver::get_upper_bound(){

    srand(time(NULL));

    int num_iterations = 1;

    while(num_iterations > 0){
        printDebug("start heuristic iteration... ");
        g->reset_graph();        // reset graph to its original
        random_cluster_graph();  // greedy cluster graph initialization, returns solution size k
        localSearch();  // local search until minimum is reached, returns improvement in k
        save_best_solution();    // save computed solution if its better than the best one
        num_iterations --;
    }
    g->reset_graph();
    return best_k;
}



void Solver::save_into_best_solution_stack(std::stack<WCE_Graph::stack_elem> current_stack){
    while(!best_solution_stack.empty()){
        best_solution_stack.pop();
    }
    while(!current_stack.empty()){
        best_solution_stack.push(current_stack.top());
        current_stack.pop();
    }
}


// outputs edges in "best_solution_heuristic"
void Solver::output_from_best_solution_stack(){
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
    verify_cluster_graph();
    clear_stack_and_output();
}

