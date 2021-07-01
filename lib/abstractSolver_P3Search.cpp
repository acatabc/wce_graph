
#include <tuple>
#include <algorithm>
#include <map>
#include "AbstractSolver.h"
#include "../include/utils.h"

// ----------------------------
// ------- p3 - search --------


AbstractSolver::p3 AbstractSolver::generate_p3_struct(int i, int j, int k) {
    int weight_ij = g->get_weight(i, j);
    int weight_ik = g->get_weight(i, k);
    int weight_jk = g->get_weight(j, k);

    if (weight_ij >= 0 && weight_ik >= 0 && weight_jk <= 0) {
        // sum up costs of all three edges (only edges that are allowed to be modified)
        int cost_sum = 0;
        if (weight_ik != DO_NOT_DELETE && weight_ik != DO_NOT_ADD) cost_sum += abs(weight_ik);
        if (weight_ij != DO_NOT_DELETE && weight_ij != DO_NOT_ADD) cost_sum += abs(weight_ij);
        if (weight_jk != DO_NOT_DELETE && weight_jk != DO_NOT_ADD) cost_sum += abs(weight_jk);

        // get minimum edge cost
        int min_cost = INT32_MAX;
        if (weight_ik != DO_NOT_DELETE && weight_ik != DO_NOT_ADD &&
            abs(weight_ik) < min_cost)
            min_cost = abs(weight_ik);
        if (weight_ij != DO_NOT_DELETE && weight_ij != DO_NOT_ADD &&
            abs(weight_ij) < min_cost)
            min_cost = abs(weight_ij);
        if (weight_jk != DO_NOT_DELETE && weight_jk != DO_NOT_ADD &&
            abs(weight_jk) < min_cost)
            min_cost = abs(weight_jk);

        AbstractSolver::p3 newP3 = {.i = i, .j = j, .k = k, .cost_sum = cost_sum, .min_cost = min_cost};
        return newP3;
    }
    else{
        AbstractSolver::p3 no_p3 = {.i = -1, .j = -1, .k = -1, .cost_sum = -1, .min_cost = -1};
        return no_p3;
    }
}



// iterates over all vertex tuples and returns max_cost p3
AbstractSolver::p3 AbstractSolver::get_max_cost_p3(){

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
    return AbstractSolver::p3{.i = u, .j = v, .k = w, .cost_sum = -1 ,.min_cost = -1};
}



// comparator returns p3 with higher minimum edge cost
bool compareP3_min_cost(AbstractSolver::p3& a, AbstractSolver::p3& b){
    return a.min_cost > b.min_cost;
}
// comparator returns p3 with higher sum of edge cost
bool compareP3_sum_cost(AbstractSolver::p3& a, AbstractSolver::p3& b){
    return a.cost_sum > b.cost_sum;
}

// iterates over all vertex tuples, returns max_cost p3, as well as a greedy lower bound
std::tuple<AbstractSolver::p3, int> AbstractSolver::get_best_p3_and_lower_bound(int heuristic, int version){
    int lower_bound = 0;

    AbstractSolver::p3 best_p3 = AbstractSolver::p3{.i = -1, .j = -1, .k = -1, .cost_sum = -1, .min_cost = -1};

    // init edge disjoint map: 1 means edge is contained in some p3 whose min edge has been counted (0 not)
    std::vector<std::vector<int>> edge_disjoint_map = std::vector<std::vector<int>>(g->merge_map.size());
    for(int i = 0; i< g->merge_map.size(); i++){
        for(int j = 0; j< g->merge_map.size(); j++){
            edge_disjoint_map[i].push_back(0);
        }
    }

    switch (version) {
        case LOWER_BOUND_FAST: {
            int max_cost = INT32_MIN;
            for (int i: this->g->active_nodes) {
                for (int j: this->g->active_nodes) {
                    for (int k : this->g->active_nodes) {
                        if (i == j || i == k || k == j) continue;
                        int weight_i_j = g->get_weight(i, j);
                        int weight_i_k = g->get_weight(i, k);
                        int weight_j_k = g->get_weight(j, k);
                        if (weight_i_j >= 0 && weight_i_k >= 0 && weight_j_k <= 0) {

                            // sum up costs of all three edges (only edges that are allowed to be modified)
                            int current_cost = 0;
                            if (weight_i_k != DO_NOT_DELETE && weight_i_k != DO_NOT_ADD)
                                current_cost += abs(weight_i_k);
                            if (weight_i_j != DO_NOT_DELETE && weight_i_j != DO_NOT_ADD)
                                current_cost += abs(weight_i_j);
                            if (weight_j_k != DO_NOT_DELETE && weight_j_k != DO_NOT_ADD)
                                current_cost += abs(weight_j_k);

                            // update maximum cost and corresponding p3
                            if (current_cost > max_cost) {
                                max_cost = current_cost;
                                best_p3.i = i;
                                best_p3.j = j;
                                best_p3.k = k;
                            }

                            // if this is an edge disjoint p3, increase lower bound
                            if (edge_disjoint_map[i][j] == 0 && edge_disjoint_map[j][k] == 0 &&
                                edge_disjoint_map[i][k] == 0) {
                                int min_cost = INT32_MAX;
                                if (weight_i_k != DO_NOT_DELETE && weight_i_k != DO_NOT_ADD &&
                                    abs(weight_i_k) < min_cost)
                                    min_cost = abs(weight_i_k);
                                if (weight_i_j != DO_NOT_DELETE && weight_i_j != DO_NOT_ADD &&
                                    abs(weight_i_j) < min_cost)
                                    min_cost = abs(weight_i_j);
                                if (weight_j_k != DO_NOT_DELETE && weight_j_k != DO_NOT_ADD &&
                                    abs(weight_j_k) < min_cost)
                                    min_cost = abs(weight_j_k);

                                lower_bound += min_cost;

                                edge_disjoint_map[i][j] = 1;
                                edge_disjoint_map[j][i] = 1;
                                edge_disjoint_map[j][k] = 1;
                                edge_disjoint_map[k][j] = 1;
                                edge_disjoint_map[i][k] = 1;
                                edge_disjoint_map[k][i] = 1;
                            }
                        }
                    }
                }
            }
            printDebug("lower bound: " + std::to_string(lower_bound));
            break;
        }
        case LOWER_BOUND_IMPROVED:

            std::vector<AbstractSolver::p3> allP3 = find_all_p3();
            if ((allP3).empty()) return std::make_tuple(best_p3, 0);

            // use sorted p3 list to choose edge disjoint p3 in helpful order
            std::sort((allP3).begin(), (allP3).end(), compareP3_min_cost);

            // to get max_sum_edge_cost p3
            int max_cost_sum = -1;
            int arg_max = -1;


            // compute lower bound
            int l = 0;
            for (auto &p3: allP3) {
                int i = p3.i;
                int j = p3.j;
                int k = p3.k;

                // if this is an edge disjoint p3, increase lower bound
                if (edge_disjoint_map[i][j] == 0 && edge_disjoint_map[j][k] == 0 && edge_disjoint_map[i][k] == 0) {
                    lower_bound += p3.min_cost;

                    edge_disjoint_map[i][j] = 1;
                    edge_disjoint_map[j][i] = 1;
                    edge_disjoint_map[j][k] = 1;
                    edge_disjoint_map[k][j] = 1;
                    edge_disjoint_map[i][k] = 1;
                    edge_disjoint_map[k][i] = 1;

                }

                if (heuristic == MAX_SUM_P3) {
                    // find p3 with max edge cost sum
                    if (p3.cost_sum > max_cost_sum) {
                        arg_max = l;
                        max_cost_sum = p3.cost_sum;
                    }
                    l++;
                }
            }

            // HEURISTIC
            // 0: max_sum_edge_cost p3
            // 1: max_min_edge_cost p3

            if (heuristic == MAX_SUM_P3)
                best_p3 = AbstractSolver::p3{.i = allP3[arg_max].i, .j = allP3[arg_max].j, .k = allP3[arg_max].k, .cost_sum = -1, .min_cost = -1};
            if (heuristic == MAX_MIN_EDGE_P3)
                best_p3 = AbstractSolver::p3{.i = (allP3)[0].i, .j = (allP3)[0].j, .k = (allP3)[0].k, .cost_sum = -1, .min_cost = -1 };
            break;
    }
    return std::make_tuple(best_p3, lower_bound);
}



std::vector<AbstractSolver::p3> AbstractSolver::find_all_p3() {

    bool *already_checked = new bool[g->merge_map.size()];
    for(int i = 0; i < g->merge_map.size(); ++i){
        already_checked[i] = false;
    }
    std::vector<AbstractSolver::p3> all_p3;
    for(int start_node : g->active_nodes){ // O(n)
        //BFS algorithm here
        std::vector<bool> visited(g->merge_map.size(), false);
        std::vector<int> queue;
        queue.push_back(start_node);
        visited[start_node] = true;
        int layer = 1;
        int current_node;
        int nodes_in_layer = queue.size();

        while(!queue.empty()){
            if(nodes_in_layer == 0){
                nodes_in_layer = queue.size();
                layer++;
                if(layer > 2)
                    break;
            }
            current_node = queue.front();
            //pop front
            queue.erase(queue.begin());
            //go through all neighbours of the current node that are not visited yet and add them to the queue O(n) / O(m)
            for(int i : g->active_nodes){
                if(i == current_node) continue;
                if(g->get_weight(current_node,i) > 0 && (!visited[i])){
                    queue.push_back(i);
                    if(layer < 2){
                        visited[i] = true;
                    }else if(layer == 2){
                        if(!already_checked[i]){
                            auto new_p3 = generate_p3_struct(current_node,start_node,i);
                            all_p3.push_back(new_p3);
                        }
                    }
                }
            }
            nodes_in_layer--;
        }
        already_checked[start_node] = true;
    }
    delete[] already_checked;
    return all_p3;
}
