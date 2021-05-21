
#include "Solver.h"
#include <tuple>
#include "iostream"
#include "../include/utils.h"
#include <math.h>

#define FORWARD 0
#define BACKWARD 1

//const char* FILENAME = "../wce-students/2-real-world/w017.dimacs";
const char* FILENAME = "../../wce-students-real/2-real-world/w088.dimacs";
//const char* FILENAME = "../test_data/w001.dimacs";

#define NONE -1
#define CLUSTER_GRAPH -2

Solver::Solver(){
    this->g = this->parse_and_build_graph();

}

Solver::~Solver() {}

void Solver::solve() {

    // apply data reduction before branching
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

    int cost_before_branching = INT32_MAX - k_tmp;
    int stack_before_branching = g->graph_mod_stack.size();

    int k = 0;
    while (true){
        printDebug("\nSOLVE FOR k:" + std::to_string(k));

        int k_reduced = k - cost_before_branching;

        k_reduced = data_reduction(k_reduced, 0);

        if(this->branch(k_reduced, 0) == CLUSTER_GRAPH){
            final_unmerge_and_output();
            break;
        }
        undo_data_reduction(stack_before_branching);
        k++;
    }

    verify_clusterGraph(); // only used in debug

    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(k) + "\n");
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



int Solver::branch(int k, int layer){
    if(k < 0){
        return NONE;
    }


    auto p3 = this->get_max_cost_p3_naive();


    if(std::get<0>(p3) == -1){
        printDebug("FOUND CLUSTER GRAPH");
        return CLUSTER_GRAPH;
    }
    rec_steps++;

    int v = std::get<0>(p3);
    int w = std::get<1>(p3);
    int u = std::get<2>(p3);
//    printDebug(std::to_string(layer) + ": " + std::to_string(v) + " "+ std::to_string(w) + " "+ std::to_string(u));
    if(this->branchEdge(u,v,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(v,w,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(w,u,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;

    return NONE;
}

int Solver::branchEdge(int u, int v, int k, int layer){
//    printDebug("Layer " + std::to_string(layer) + " Edge " + std::to_string(u) + ", " + std::to_string(v));
    int weight = g->get_weight(u,v);
//
    if(weight == DO_NOT_DELETE || weight == DO_NOT_ADD) {
//        printDebug("Do not delete / add " + std::to_string(u) + ", " + std::to_string(v));
        return NONE;
    }

    // if weight = 0, use flag to branch once with deleting edge and once with adding edge
    bool flag_branch_zero = true;
    branch_zero:
    if(weight == 0){
        if(flag_branch_zero == true) g->add_edge(u, v);
        if(flag_branch_zero == false) g->delete_edge(u, v);
    }

    if(weight > 0) g->delete_edge(u, v);
    if(weight < 0) g->add_edge(u, v);

    int prev_stack_size = g->graph_mod_stack.size(); // save stack size to recover current graph after data reduction


    int k_reduced = k-abs(weight);


    // if {u,v} < 0, we inserted edge ==> merge {u,v}
    if(weight < 0){
        int cost = g->merge(u,v);
        if(cost == -1) k_reduced = -1;
        else k_reduced -= cost;
    }


    k_reduced = data_reduction(k_reduced, layer);

    if(this->branch(k_reduced, layer) == CLUSTER_GRAPH){
        if(u < g->num_vertices && v < g->num_vertices)
            final_output(u,v);
        else
            printDebug("output (later): " + std::to_string(u+1-1) + " " + std::to_string(v+1-1));
        return CLUSTER_GRAPH;
    }

    undo_data_reduction(prev_stack_size);
    g->set_weight(u, v, weight);

    // redo branching for edge weight 0 with deleting edge
    if(weight == 0 && flag_branch_zero == true){
        flag_branch_zero = false;
        goto branch_zero;
    }

    return NONE;
}

void Solver::final_output(int u, int v)
{
#ifdef DEBUG
    if(PRINTDEBUG){
//        std::cout << u+1-1 << " " << v+1-1 <<std::endl;
    }
#endif
    std::cout << u+1 << " " << v+1 <<std::endl;
}


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
#ifdef DEBUG
    //    std::cout << "counter " << counter<< " Found " << p3_list.size() << " p3's"<< std::endl;
#endif
    return std::make_tuple(first_tuple_val, second_tuple_val, third_tuple_val);
}




// ----------------------------
// ------- data reduction  --------

int Solver::data_reduction(int k, int layer){
    int k_before = k;
    // try different values for layers
    if(layer %5 ==  0 && layer >= 10){
////    this->dataRed_heavy_non_edge();
////    k = dataRed_heavy_edge_single_end(k);
//        dataRed_remove_existing_clique();
//        k = dataRed_heavy_non_edge_branch(k);
//        k = dataRed_heavy_edge_single_end_branch(k);
//        k = dataRed_large_neighbourhood_I(k);
        k = dataRed_heavy_edge_both_ends(k);
    }

    if(layer % 1 == 0){
        k = dataRed_weight_larger_k(k);
//        k = dataRed_heavy_edge_single_end_branch(k);
//        k = dataRed_large_neighbourhood_I(k);

    }
//    if(layer % 3 == 0){
//        k = dataRed_heavy_edge_both_ends(k);
//        k = dataRed_heavy_non_edge_branch(k);
//
//    }
    if(k != k_before)
        printDebug("Data reduction reduced k to " + std::to_string(k));
    return k;
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


            g->graph_mod_stack.push(WCE_Graph::stack_elem{.type = 3, .v1 = -1, .v2 = -1, .weight = -1, .uv = -1, .clique = component});

//            g->graph_mod_stack.push(WCE_Graph::stack_elem{.type = 3, .clique = component});

        }
//        std::cout << std::endl;
    }

//    for(int i : g->active_nodes){
//        std::cout << i + 1 << " ";
//    }

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

// ----------------------------
// ------- merging --------


// unmerges all remaining vertices and outputs edges that had to be modified for merging
void Solver::final_unmerge_and_output(){
    int k = 0;
    while (g->graph_mod_stack.size() != 0){
//        verify_clusterGraph();
        WCE_Graph::stack_elem el = g->graph_mod_stack.top();
        if(el.type == 1) {
            int uv = el.uv;
            printDebug("unmerge " + std::to_string(uv));
            int dk = unmerge_and_output(uv);
            k += dk;
        }
        else if(el.type == 2) {
            g->graph_mod_stack.pop();
        }
        else if(el.type == 3){
            for(int i: el.clique){
                g->active_nodes.push_back(i);
            }
            g->graph_mod_stack.pop();
        }
    }
    printDebug("\nUnmerging sum of costs " +  std::to_string(k));
}

// unmerges vertex uv and outputs all edges {x,u} and {x,v} that had to be modified for merging
int Solver::unmerge_and_output(int uv){
    std::vector<int> uv_children = g->merge_map[uv];

    printDebug("Output for unmerging " + std::to_string(uv) + " -> (" + std::to_string(uv_children[0]) + "," + std::to_string(uv_children[1]) + ")");

    int dk = 0;
    for(int x : g->active_nodes) {
        if(x == uv || x == uv_children[0] ||x == uv_children[1]) continue;

        int weight_uvx = g->get_weight(uv, x);

        if(weight_uvx == 0){
            // if {uv,x} = 0,  it doesnt matter if we propagate "delete" or "add" (choose delete)
            printDebug("Propagate : " + std::to_string(uv) + " " + std::to_string(x) + " weight " + std::to_string(weight_uvx) + "  -> choose delete" );
            g->delete_edge(uv,x);
            weight_uvx = g->get_weight(uv, x);
        }

        for(int u: uv_children){
            int weight_ux = g->get_weight(u, x);

            // adapt edge {u,x} to previous merged edge {uv,x}
            // we have to modify edge {u,x} if {u,x} = 0 and if {u,x} and {uv,x} have different sign
            printDebug("Propagate : " + std::to_string(uv) + " " + std::to_string(x) + " weight " + std::to_string(weight_uvx) + "    to    " + std::to_string(u) + " " + std::to_string(x) + " weight " + std::to_string(weight_ux) );
            if (weight_ux == 0 || (signbit(weight_ux) != signbit(weight_uvx))) {

                if(weight_uvx > 0)
                    g->add_edge(u,x);
                if(weight_uvx < 0)
                    g->delete_edge(u,x);

                if(u < g->num_vertices && x < g->num_vertices)
                    final_output(u,x);
                else
                    printDebug("Output queue: " + std::to_string(u+1-1) + " " + std::to_string(x+1-1));

                dk += abs(weight_ux); // debug
            }
        }
    }

    g->undo_final_modification();
    printDebug("Unmerged " +  std::to_string(uv) + " -> (" + std::to_string(uv_children[0]) + "," + std::to_string(uv_children[1]) + ")" + " with cost " + std::to_string(dk));

    return dk;
}


WCE_Graph *Solver::parse_and_build_graph(){

#ifdef DEBUG
    //    freopen("../wce-students/2-real-world/w027.dimacs", "r", stdin);
//    freopen("../test_data/r049.dimacs", "r", stdin);
    freopen(FILENAME, "r", stdin);
#endif
    int num_vertices = 0;
    std::cin >> num_vertices;
    WCE_Graph *g = new WCE_Graph(num_vertices);
    int v, w, weight;
    while(std::cin){
        std::cin >> v >> w >> weight;
        v -= 1;
        w -= 1;
        if(!std::cin.fail())
            g->set_weight(v,w, weight);
    }
    for(int i = 0; i< g->num_vertices; i++){
        std::vector<int> u = {i};
        g->merge_map.push_back(u);
        g->active_nodes.push_back(i);
    }
    return g;
}




// ------ Debug ----


// verify that the current graph is now a cluster graph
void Solver::verify_clusterGraph(){
#ifdef DEBUG
    auto p3 = this->get_max_cost_p3_naive();
    if(std::get<0>(p3) == -1){
        printDebug("\nVERIFICATION SUCCESS\n");
    } else {
        printDebug("\nVERIFICATION FAIL:");
        print_tuple(p3);
    }
#endif
}

