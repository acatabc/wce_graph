
#include "Solver.h"
#include "../include/utils.h"

// ----------------------------
// ------- data reduction  --------

int Solver::data_reduction(int k, int layer){
    int k_before = k;

//    if(layer %5 ==  0 && layer >= 10){
////      this->dataRed_heavy_non_edge();
////      k = dataRed_heavy_edge_single_end(k);
//      k = dataRed_heavy_non_edge_branch(k);
//      k = dataRed_heavy_edge_single_end_branch(k);
//      k = dataRed_large_neighbourhood_I(k);
//      k = dataRed_heavy_edge_both_ends(k);
//    }

    k = dataRed_weight_larger_k(k);

//    if(k != k_before)
//        printDebug("Data reduction reduced k to " + std::to_string(k));
    return k;
}

// applies data reduction on current graph and data reduction cost (only used before branching)
int Solver::data_reduction_before_branching(){
    int k_tmp = INT32_MAX;
    int k_before = 0;
    dataRed_remove_existing_clique();
    while(k_tmp != k_before){
        k_before = k_tmp;
        k_tmp = dataRed_heavy_non_edge_branch(k_tmp);
        k_tmp = dataRed_heavy_edge_single_end_branch(k_tmp);
        k_tmp = dataRed_heavy_edge_both_ends(k_tmp);
        k_tmp = dataRed_large_neighbourhood_I(k_tmp);
        k_tmp = dataRed_merge_dnd(k_tmp);
    }
    int cost = INT32_MAX - k_tmp;
    return cost;
}


void Solver::undo_data_reduction(int prev_stack_size){
    while (g->graph_mod_stack.size() != prev_stack_size){
        g->undo_final_modification();
    }

}


// continuously merges all vertices whose edge weight exceeds the available costs (set edges = -inf analogously)
// returns remaining costs k after merging
// if k<0 no solution for the graph and input k exists
int Solver::dataRed_weight_larger_k(int k){
//    printDebug("Data reduction (weight > k=" + std::to_string(k) + "):");
    int k_before = k;

    redo:
    if(k < 0) {
//        printDebug("Fail: maximum cost exceeded");
        return -1;
    }
    for(int u : g->active_nodes){
        for(int v : g->active_nodes){
            if(u == v) continue;
            int weight_uv = g->get_weight(u,v);
            if(weight_uv > k){
                int cost = g->merge(u,v);
                if(cost == -1) return -1; // merging failed
                k -= cost;
                goto redo;
            }
            else if(weight_uv != DO_NOT_ADD && -weight_uv > k){
                g->set_non_edge(u,v);
            }
        }
    }
    return k;
}

int Solver::dataRed_heavy_non_edge_branch(int k) {
    redo:
    if(k < 0) {
        return -1;
    }
    for(int u : g->active_nodes){
        // compute sum of {u,w} for all active neighbors w
        int weight_neighbours = 0;
        for(int w : g->active_nodes){
            if(u == w) continue;
            int weight = g->get_weight(u,w);

            // merge if {u,w} is DND
            if(weight == DO_NOT_DELETE) {
                int cost = g->merge(u,w);
                if(cost == -1) return -1;
                k -= cost;
                goto redo;
            }

            if(weight > 0){
                weight_neighbours += weight;
            }
        }

        // compute all heavy non-edges with u
        // for all v with {u,v} < 0, check if |{u,v}| >= sum
        for(int v : g->active_nodes){
            if(u == v) continue;
            int weight_uv = g->get_weight(u,v);
            if(weight_uv == DO_NOT_ADD) // {u,v} is already a heavy non edge
                continue;
            if(weight_uv < 0 && abs(weight_uv) >= weight_neighbours){
                g->set_non_edge(u,v);
            }
        }
    }
    return k;
}

int Solver::dataRed_heavy_edge_single_end_branch(int k) {
    redo:
    if(k < 0) {
//        printDebug("Fail: maximum cost exceeded");
        return -1;
    }
    for(int u : g->active_nodes){
        // compute sum abs({u,w}) for all active neighbors w
        // save all DNA  edges of u for later; merge DND edges
        int weight_neighbours = 0;
        std::vector<int> dna = std::vector<int>();
        for(int w : g->active_nodes){
            if(u == w) continue;
            int weight = g->get_weight(u,w);

            // merge if {u,w} is DND
            if(weight == DO_NOT_DELETE) {
                int cost = g->merge(u,w);
                if(cost == -1) return -1;  // merging failed
                k -= cost;
                goto redo;
            }

            if(weight == DO_NOT_ADD)
                dna.push_back(w);
            else weight_neighbours += abs(weight);
        }

        // compute all heavy edges with u
        for(int v : g->active_nodes){
            if(u == v) continue;
            int weight_uv = g->get_weight(u,v);

            if(weight_uv < 0)
                continue;

            // merge if {u,v} is DND
            if(weight_uv == DO_NOT_DELETE){
                int cost = g->merge(u,v);
                if(cost == -1) return -1;
                k -= cost;
                goto redo;
            }

            // if {u,w} = dna, check if {w,v} agrees, otherwise add cost for changing {w,v}
            bool stop = false;
            for(int w: dna){
                if(g->get_weight(v,w) == DO_NOT_DELETE) {
                    stop = true;
                    break;
                }
                if(g->get_weight(v,w) > 0) {
                    weight_neighbours += g->get_weight(v,w);

                }
            }
            if(stop == true) continue;

            // merge if heavy edge condition holds
            if(weight_uv >= weight_neighbours - weight_uv){
                int cost = g->merge(u,v);
                if(cost == -1) return -1;
                k -= cost;
//                printDebug("Merged vertices for dataRed_heavy_edge_single_end: (" + std::to_string(u) + "," + std::to_string(v) + ")" );
                goto redo;
            }
        }
    }

    return k;
}

void Solver::dataRed_heavy_non_edge() {
    for(int i : g->active_nodes){
        int min_val = 0;
        int i_min = -1;
        int j_min = -1;
        int weight_neighbours = 0;
        for(int j : g->active_nodes){
            if(i == j) continue;
            int weight = g->get_weight(i,j);
            if(weight < 0 && weight < min_val){
                min_val = weight;
                i_min = i;
                j_min = j;
            }else if(weight > 0){
                weight_neighbours += weight;
            }
        }
        if(abs(min_val) >= weight_neighbours && i_min != -1 && weight_neighbours > 0){
            g->set_non_edge(i_min,j_min);
            printDebug("set edge infinity for dataRed_heavy_non_edge");
        }
    }
    return;
}

int Solver::dataRed_heavy_edge_single_end(int k) {
    int cost = 0;
    redo:
    if(k < 0) {
//        printDebug("Fail: maximum cost exceeded");
        return -1;
    }
    for(int i : g->active_nodes){
        int max_weight = 0;
        int sum = 0;
        int i_max = -1;
        int j_max = -1;
        for(int j : g->active_nodes){
            if(i == j) continue;
            int weight = g->get_weight(i,j);
            if(weight == DO_NOT_DELETE){
                int cost = g->merge(i,j);
                if(cost == -1) return -1; // merging failed
                k -= cost;
                goto redo;
            }

            if(weight > max_weight){
                max_weight = weight;
                i_max = i;
                j_max = j;
            }
            if(weight != DO_NOT_ADD && weight != DO_NOT_DELETE && sum != DO_NOT_DELETE && sum != DO_NOT_ADD)
                sum += abs(weight);
            else{
                sum = DO_NOT_DELETE; // abs(do_not_add) is Do not add again => smaller 0
                break;
            }
        }

        if(max_weight >= sum - max_weight && i_max != -1) {
            int cost = g->merge(i_max,j_max);
            if(cost == -1) return -1; // merging failed
            k -= cost;
            goto redo;
        }
    }
    if(cost > 0) printDebug("Merged vertices for dataRed_heavy_edge_single_end, cost: " + std::to_string(cost));
    return k;
}

int Solver::dataRed_heavy_edge_both_ends(int k) {
    int cost = 0;
    redo:
    if(k < 0) {
//        printDebug("Fail: maximum cost exceeded");
        return -1;
    }
    for( int u: g->active_nodes){
        int max_weight = 0;
        int max_u = -1;
        int max_v = -1;
        for(int v : g->active_nodes){
            if(u == v) continue;
            int _weight = g->get_weight(u,v);
            if(_weight > max_weight){
                max_weight = _weight;
                max_u = u;
                max_v = v;
            }
        }
        if(max_u != -1){
            auto neighbours_of_u = closed_neighbourhood(max_u).first;
            auto neighbours_of_v = closed_neighbourhood(max_v).first;
            int sum_of_u = 0;
            int sum_of_v = 0;
            for(auto i: neighbours_of_u){
                if(i == max_u || i == max_v) continue;
                int weight = g->get_weight(i,max_u);
                if(weight != DO_NOT_DELETE && weight != DO_NOT_ADD)
                    sum_of_u += g->get_weight(i,max_u);
                else{
                    sum_of_u = DO_NOT_DELETE;
                    break;
                }
            }

            for(auto i : neighbours_of_v) {
                if (i == max_u || i == max_v) continue;
                int weight = g->get_weight(i, max_v);
                if(weight != DO_NOT_DELETE && weight != DO_NOT_ADD)
                    sum_of_v += g->get_weight(i, max_v);
                else{
                    sum_of_v = DO_NOT_DELETE;
                    break;
                }
            }

            int sum = sum_of_u + sum_of_v;
            if(sum_of_u > 0 && sum_of_v > 0 && sum < 0){
                sum = DO_NOT_DELETE;
            }
            if(sum_of_u < 0 && sum_of_v < 0 && sum > 0){
                sum = DO_NOT_ADD;
            }
            if(max_weight >= sum){
                int cost = g->merge(max_u, max_v);
                if(cost == -1) return -1;
                k -= cost;
                printDebug("Successful heavy_edge_both_ends merge costs: " + std::to_string(cost));
                goto redo;
            }
        }

    }

    return k;
}

int Solver::dataRed_remove_existing_clique() {
    int N = g->merge_map.size();
    bool *visited = new bool[N];

    for(int i = 0; i < N; ++i){
        visited[i] = false;
    }
    std::vector<std::vector<int>> components;
    for(int i : g->active_nodes){
        if(visited[i] == false){
            components.push_back({});
            DFS(i, visited, components.back());
        }
    }

    for(auto &component : components){
        bool is_clique = true;
        for(int i = 0; i < component.size() && is_clique; ++i){
//            std::cout << component.at(i) + 1 << " ";
            for(int j = i +1 ; j < component.size(); ++j){
                // for {u,v} = 0 we consider u,v as not adjacent (we delete {u,v} later in unmerging)
                // thus, we have a clique only if {u,v} > 0
                if(g->get_weight(component.at(i), component.at(j)) <= 0){
                    is_clique = false;
                    break;
                }

            }
        }
        if(is_clique){
//            std:: cout << " is a clique" << std::endl;
            for(auto i = g->active_nodes.begin(); i != g->active_nodes.end(); ++i){
                for(int j : component){
                    if(*i == j){
                        i = g->active_nodes.erase(i);
                        i--;
                    }
                }
            }
            g->graph_mod_stack.push(WCE_Graph::stack_elem{.type = CLIQUE, .v1 = -1, .v2 = -1, .weight = -1, .uv = -1, .clique = component});
        }
    }

    delete[] visited;
    return 0;
}


// merges all vertices with DND=INF edges
int Solver::dataRed_merge_dnd(int k){
//    printDebug("Data reduction merge DND" + std::to_string(k) + "):");
    int k_before = k;

    redo:
    if(k < 0) {
        printDebug("Fail: maximum cost exceeded");
        return -1;
    }
    for(int u : g->active_nodes){
        for(int v : g->active_nodes){
            if(u == v) continue;
            if(g->get_weight(u,v) == DO_NOT_DELETE){
                int cost = g->merge(u,v);
                if(cost == -1) return -1; // merging failed
                k -= cost;
                goto redo;
            }
        }
    }

    if(k != k_before)
        printDebug("Data reduction merge DND reduced k to " + std::to_string(k));
//    else
//        printDebug("no edges to merge");

    return k;
}

void Solver::DFS(int i, bool *visited, std::vector<int>& component) {
    visited[i] = true;
//    std::cout << i+1 << " ";
    component.push_back(i);
    for(int j : g->active_nodes){
        if(i == j) continue;
        // for {i,j} = 0 we consider i,j as not adjacent (we delete {i,j} later in unmerging)
        // thus, i and j are in different components if {i,j} <= 0
        if(g->get_weight(i,j) > 0){
            if(visited[j] == false){
                DFS(j, visited, component);
            }
        }
    }
    return;
}
//is doing the large Neighbourhood Rule for all vertices in the graph TODO
int Solver::dataRed_large_neighbourhood_I(int k) {

    int merge_costs = 0;
    rerun_after_merge:
    if(k < 0) {
//        printDebug("Fail: maximum cost exceeded");
        return -1;
    }
    for(int i : g->active_nodes){
        auto neighbourhoods = closed_neighbourhood(i);
        auto neighbours = neighbourhoods.first;
        auto not_neighbours = neighbourhoods.second;
        int deficiency = this->deficiency(neighbours);
        int cut_weight = this->cut_weight(neighbours, not_neighbours);
        if(deficiency != DO_NOT_DELETE && cut_weight != DO_NOT_DELETE) {
            int sum = 2 * deficiency + cut_weight;
            if(sum < 0) // int overflow
                sum = DO_NOT_DELETE;
            if(sum < neighbours.size() && neighbours.size() >= 2){
                int first = neighbours.front();
                neighbours.pop_front();
                int second = neighbours.front();
                neighbours.pop_front();

                int val = g->merge(first,second);
                if(val == -1) return -1; // merging failed
                merge_costs += val;

                while(!neighbours.empty()){
                    int last_merged = g->active_nodes.back();
                    int next_from_neighbourhood = neighbours.front();
                    neighbours.pop_front();

                    int val = g->merge(last_merged, next_from_neighbourhood);
                    if(val == -1) return -1; // merging failed
                    merge_costs += val;

                }
                printDebug("Successful large neighborhood 1, merge costs: " + std::to_string(merge_costs));
                goto rerun_after_merge;

            }
        }
    }
    k -= merge_costs;
    return k;
}

// param: u is the index of the vertex of which the neighbours are collected
// return: - pair for neighbourhood(first item in pair) - all the vertices that are adjacent to u,
//         - not_neighbours(second item in pair) - all vertices that are not adjacent to u
//
std::pair<std::list<int>, std::list<int>> Solver::closed_neighbourhood(int u) {
    std::list<int> neighbours;
    std::list<int> not_neighbours;
    for(int i : g->active_nodes){
        if(i == u) continue;
        if(g->get_weight(u,i) > 0){
            neighbours.push_back(i);
        }else if(g->get_weight(u,i) < 0){
            not_neighbours.push_back(i);
        }
    }
    neighbours.push_back(u);
    return std::make_pair(neighbours, not_neighbours);
}

//calculates the costs to make the neighbourhood a clique
int Solver::deficiency(std::list<int> neighbours) {
    int costs = 0;
    while (!neighbours.empty()) {
        int i = neighbours.front();
        neighbours.pop_front();
        for (int j : neighbours) {
            if ( i == j) continue;
            if (g->get_weight(i, j) < 0) {
                if(g->get_weight(i,j) == DO_NOT_ADD){
                    return DO_NOT_DELETE; // abs(DO_NOT_ADD) is DO_NOT_ADD again but a high value should be returned
                }
                costs += abs(g->get_weight(i, j));
            }
        }
    }
    return costs;
}

//calculates the cost to cut of the neighbourhood(neighbourhood) from the rest of the graph(rest_graph)
int Solver::cut_weight(std::list<int>& neighbourhood, std::list<int>& rest_graph) {
    int cut_costs = 0;
    for(int i : neighbourhood){
        for(int j : rest_graph){
            if(i == j)continue;
            int weight = g->get_weight(i,j);
            if(weight > 0){
                if(weight == DO_NOT_DELETE){
                    return DO_NOT_DELETE;
                }
                cut_costs += weight;
            }
        }
    }
    return cut_costs;
}



void Solver::output_data_reduction() {
    g->printGraph(std::cout);

    int k_tmp = INT32_MAX;
    int k_before = 0;
    while(k_tmp != k_before){
        k_before = k_tmp;
        dataRed_remove_existing_clique();
        k_tmp = dataRed_heavy_non_edge_branch(k_tmp);
        k_tmp = dataRed_heavy_edge_single_end_branch(k_tmp);
        k_tmp = dataRed_heavy_edge_both_ends(k_tmp);
        k_tmp = dataRed_large_neighbourhood_I(k_tmp);
        k_tmp = dataRed_merge_dnd(k_tmp);
    }

    int cost = INT32_MAX - k_tmp;

    std::cout << g->active_nodes.size() << "\n";
    int i = 0;
    for(int u: g->active_nodes){
        int j = 0;
        for(int v: g->active_nodes){
            if(u < v){
                int weight = g->get_weight(u,v);
                if(weight == DO_NOT_ADD){
                    std::cout << i+1 << " " << j+1 << " " << "-inf" << "\n";
                }else{
                    std::cout << i+1 << " " << j+1 << " " << g->get_weight(u,v) << "\n";
                }
            }
            j++;
        }
        i++;
    }

    std::cout << "#weight:" << cost << "\n";

}