
#include <tuple>
#include <algorithm>
#include <map>
#include "Solver.h"
#include "../include/utils.h"
#define HEURISTIC 1

// ----------------------------
// ------- p3 - search --------

// iterates over all vertex tuples and returns max_cost p3
std::tuple<int, int, int> Solver::get_max_cost_p3_naive(){

    int first_tuple_val = -1;
    int second_tuple_val = -1;
    int third_tuple_val = -1;
    int max_cost = INT32_MIN;
    for(int i: this->g->active_nodes){
        for(int j: this->g->active_nodes){
            for(int k : this->g->active_nodes){
                if(i == j || i == k || k == j) continue;
                int weight_i_j = g->get_weight(i,j);
                int weight_i_k = g->get_weight(i,k);
                int weight_j_k = g->get_weight(j,k);
                if(weight_i_j >= 0 && weight_i_k >= 0 && weight_j_k <= 0){

                    // sum up costs of all three edges (only edges that are allowed to be modified)
                    int current_cost = 0;
                    if(weight_i_k != DO_NOT_DELETE && weight_i_k != DO_NOT_ADD) current_cost += abs(weight_i_k);
                    if(weight_i_j != DO_NOT_DELETE && weight_i_j != DO_NOT_ADD) current_cost += abs(weight_i_j);
                    if(weight_j_k != DO_NOT_DELETE && weight_j_k != DO_NOT_ADD) current_cost += abs(weight_j_k);

                    // update maximum cost and corresponding p3
                    if(current_cost > max_cost) {
                        max_cost = current_cost;
                        first_tuple_val = i;
                        second_tuple_val = j;
                        third_tuple_val = k;
                    }
                }
            }

        }
    }
    return std::make_tuple(first_tuple_val, second_tuple_val, third_tuple_val);
}

// iterates over all vertex tuples, returns max_cost p3, as well as a greedy lower bound
std::tuple<std::tuple<int, int, int>, int> Solver::get_max_cost_p3_naive_lowerBound(){

    int lower_bound = 0;

    // init edge disjoint map: 1 means edge is contained in some p3 which has been counted (0 not)
    std::vector<std::vector<int>> edge_disjoint_map = std::vector<std::vector<int>>(g->merge_map.size());
    for(int i = 0; i< g->merge_map.size(); i++){
        for(int j = 0; j< g->merge_map.size(); j++){
            edge_disjoint_map[i].push_back(0);
        }
    }

    int first_tuple_val = -1;
    int second_tuple_val = -1;
    int third_tuple_val = -1;
    int max_cost = INT32_MIN;
    for(int i: this->g->active_nodes){
        for(int j: this->g->active_nodes){
            for(int k : this->g->active_nodes){
                if(i == j || i == k || k == j) continue;
                int weight_i_j = g->get_weight(i,j);
                int weight_i_k = g->get_weight(i,k);
                int weight_j_k = g->get_weight(j,k);
                if(weight_i_j >= 0 && weight_i_k >= 0 && weight_j_k <= 0){

                    // sum up costs of all three edges (only edges that are allowed to be modified)
                    int current_cost = 0;
                    if(weight_i_k != DO_NOT_DELETE && weight_i_k != DO_NOT_ADD) current_cost += abs(weight_i_k);
                    if(weight_i_j != DO_NOT_DELETE && weight_i_j != DO_NOT_ADD) current_cost += abs(weight_i_j);
                    if(weight_j_k != DO_NOT_DELETE && weight_j_k != DO_NOT_ADD) current_cost += abs(weight_j_k);

                    // update maximum cost and corresponding p3
                    if(current_cost > max_cost) {
                        max_cost = current_cost;
                        first_tuple_val = i;
                        second_tuple_val = j;
                        third_tuple_val = k;
                    }

                    // if this is an edge disjoint p3, increase lower bound
                    if(edge_disjoint_map[i][j] == 0 && edge_disjoint_map[j][k] == 0 && edge_disjoint_map[i][k] == 0) {
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
    return std::make_tuple(std::make_tuple(first_tuple_val, second_tuple_val, third_tuple_val), lower_bound);
}

// comparator returns p3 with higher minimum edge cost
bool compareP3_min_cost(Solver::p3& a, Solver::p3& b){
    return a.min_cost > b.min_cost;
}
// comparator returns p3 with higher sum of edge cost
bool compareP3_sum_cost(Solver::p3& a, Solver::p3& b){
    return a.cost_sum > b.cost_sum;
}

// returns the best of all p3 based on heuristic (max_min_edge_cost / max_sum_edge_cost) and an (improved) lower bound
std::tuple<std::tuple<int, int, int>, int> Solver::get_best_p3_and_lowerBound_improved(){

    std::vector<Solver::p3> allP3 = find_all_p3_faster();
    if((allP3).empty()) return std::make_tuple(std::make_tuple(-1,-1,-1), 0);

    // use sorted p3 list to choose edge disjoint p3 in helpful order
    std::sort((allP3).begin(), (allP3).end(), compareP3_min_cost);


    // to get max_sum_edge_cost p3
    int max_cost_sum = -1;
    int arg_max = -1;

    // init edge disjoint map: 1 means edge is contained in some p3 whose min edge has been counted (0 not)
    std::vector<std::vector<int>> edge_disjoint_map = std::vector<std::vector<int>>(g->merge_map.size());
    for(int i = 0; i< g->merge_map.size(); i++){
        for(int j = 0; j< g->merge_map.size(); j++){
            edge_disjoint_map[i].push_back(0);
        }
    }

    // compute lower bound
    int lower_bound = 0;
    int l = 0;
    for(auto& p3: allP3){
        int i = p3.i;
        int j = p3.j;
        int k = p3.k;

        // if this is an edge disjoint p3, increase lower bound
        if(edge_disjoint_map[i][j] == 0 && edge_disjoint_map[j][k] == 0 && edge_disjoint_map[i][k] == 0) {
            lower_bound += p3.min_cost;

            edge_disjoint_map[i][j] = 1;
            edge_disjoint_map[j][i] = 1;
            edge_disjoint_map[j][k] = 1;
            edge_disjoint_map[k][j] = 1;
            edge_disjoint_map[i][k] = 1;
            edge_disjoint_map[k][i] = 1;

        }

        if(HEURISTIC == 0) {
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
    std::tuple<int,int,int> best_p3;
    if(HEURISTIC == 0)  best_p3 = std::make_tuple(allP3[arg_max].i, allP3[arg_max].j, allP3[arg_max].k);
    if(HEURISTIC == 1)  best_p3 = std::make_tuple((allP3)[0].i, (allP3)[0].j, (allP3)[0].k);


    return std::make_tuple(best_p3, lower_bound);
}

// iterates over all vertex tuples and returns a list of p3s
std::vector<Solver::p3> Solver::find_all_p3(){
    std::vector<Solver::p3> allP3 = std::vector<Solver::p3>();

    for(int i: this->g->active_nodes){
        for(int j: this->g->active_nodes){
            for(int k : this->g->active_nodes){
                if(i == j || i == k || k == j) continue;

                auto new_p3 = generate_p3_struct(i,j,k);
                if(new_p3.i != -1)
                    allP3.push_back(new_p3);
                }
            }

        }
    return allP3;
}

Solver::p3 Solver::generate_p3_struct(int i, int j, int k) {
    int weight_i_j = g->get_weight(i,j);
    int weight_i_k = g->get_weight(i, k);
    int weight_j_k = g->get_weight(j, k);
    if (weight_i_j >= 0 && weight_i_k >= 0 && weight_j_k <= 0) {

        // sum up costs of all three edges (only edges that are allowed to be modified)
        int cost_sum = 0;
        if (weight_i_k != DO_NOT_DELETE && weight_i_k != DO_NOT_ADD) cost_sum += abs(weight_i_k);
        if (weight_i_j != DO_NOT_DELETE && weight_i_j != DO_NOT_ADD) cost_sum += abs(weight_i_j);
        if (weight_j_k != DO_NOT_DELETE && weight_j_k != DO_NOT_ADD) cost_sum += abs(weight_j_k);

        // get minimum edge cost
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

        Solver::p3 newP3 = {.i = i, .j = j, .k = k, .cost_sum = cost_sum, .min_cost = min_cost};
        return newP3;
    }else{
        Solver::p3 no_p3 = {.i = -1, .j = -1, .k = -1, .cost_sum = -1, .min_cost = -1};
        return no_p3;
    }
}
std::vector<Solver::p3> Solver::find_all_p3_faster() {

    bool *already_checked = new bool[g->merge_map.size()];
    for(int i = 0; i < g->merge_map.size(); ++i){
        already_checked[i] = false;
    }
    std::vector<Solver::p3> all_p3;
    for(int start_node : g->active_nodes){
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
            //go through all neighbours of the current node that are not visited yet and add them to the queue
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