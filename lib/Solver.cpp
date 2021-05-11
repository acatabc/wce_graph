
#include "Solver.h"
#include <tuple>
#include "iostream"
#include "../include/utils.h"
#include <math.h>


const char* FILENAME = "../wce-students/2-real-world/w037.dimacs";

#define NONE -1
#define CLUSTER_GRAPH -2

Solver::Solver(){
    this->g = this->parse_and_build_graph();
}

Solver::~Solver() {}

void Solver::solve() {
    g->printGraph(std::cout);
    verify_clusterGraph();

    int k = 140;
    int cluster_graph = NONE;
    while (cluster_graph == NONE){
        printDebug("\nSOLVE FOR k:" + std::to_string(k));

        // data reduction methods
        int k_reduced = data_reduction(k);

        verify_clusterGraph();
        g->printGraph(std::cout);

        if(k_reduced >= 0){
            cluster_graph = this->branch(k_reduced, 0);
        }

        if(cluster_graph == CLUSTER_GRAPH)
            final_unmerge_and_output();
        else{
            g->recover_original(g->num_vertices);
            k++;
            k_forward.clear();
        }

    }

    verify_clusterGraph();

    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(k) + "\n");
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

    if(this->branchEdge(u,v,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(v,w,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(w,u,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;

    return NONE;
}

int Solver::branchEdge(int u, int v, int k, int layer){
    printDebug("Layer " + std::to_string(layer) + " Edge " + std::to_string(u) + ", " + std::to_string(v));
    int weight = g->get_weight(u,v);
//
    if(weight == DO_NOT_DELETE || weight == DO_NOT_ADD) {
        printDebug("Do not delete / add " + std::to_string(u) + ", " + std::to_string(v));
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

    // data reduction
    int final_merged = g->merge_map.size(); // remember index of last vertex merged until here
    int k_reduced = data_reduction(k-abs(weight));

    if(this->branch(k_reduced, layer) == CLUSTER_GRAPH){
        if(u < g->num_vertices && v < g->num_vertices)
            final_output(u,v);
        else printDebug("output (later): " + std::to_string(u+1-1) + " " + std::to_string(v+1-1));
        return CLUSTER_GRAPH;
    }

    // undo previous merging
    while (g->merge_map.size() != final_merged){
        g->unmerge(g->merge_map.size() - 1);
    }

//    printDebug("after: " + std::to_string(g->merge_map.size()));

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
                if(this->g->get_weight(i,j) >= 0 && this->g->get_weight(i,k) >= 0 && this->g->get_weight(j,k) <= 0){

                    // sum up costs of all three edges (only edges that are allowed to be modified)
                    int current_cost = 0;
                    if(this->g->get_weight(i,k) != DO_NOT_DELETE && this->g->get_weight(i,k) != DO_NOT_ADD) current_cost += abs(g->get_weight(i,k));
                    if(this->g->get_weight(i,j) != DO_NOT_DELETE && this->g->get_weight(i,j) != DO_NOT_ADD) current_cost += abs(g->get_weight(i,j));
                    if(this->g->get_weight(j,k) != DO_NOT_DELETE && this->g->get_weight(j,k) != DO_NOT_ADD) current_cost += abs(g->get_weight(j,k));

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


int Solver::data_reduction(int k){
    int cost = dataRed_heavy_edge_single_end();
    k -= cost;
    k = dataRed_weight_larger_k(k);
    this->dataRed_heavy_non_edge();
    return k;
}

// continuously merges all vertices whose edge weight exceeds the available costs
// returns remaining costs k after merging
// if k<0 no solution for the graph and input k exists
int Solver::dataRed_weight_larger_k(int k){
    printDebug("Data reduction (weight > k=" + std::to_string(k) + "):");
    int k_before = k;

    start:
    if(k < 0) {
        printDebug("Fail: maximum cost exceeded");
        return -1;
    }
//    g->print_active_graph(std::cout);
    for(int u : g->active_nodes){
        for(int v : g->active_nodes){
            if(u == v) continue;
            if(g->get_weight(u,v) > k){ // for > -k set to -infinity TODO
                int kd = g->merge(u,v);
                if(kd < 0) continue; // merging failed
                k -= kd;
                k_forward.push_back(kd);
                goto start; // do this to avoid problems with modification of g->active_nodes TODO
            }
        }
    }

    if(k != k_before)
        printDebug("Reduced k to " + std::to_string(k));
    else
        printDebug("no edges to merge");

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
            g->set_weight(i_min, j_min, DO_NOT_ADD);
            printDebug("set edge infinity for dataRed_heavy_non_edge");
        }
    }
    return;
}


int Solver::dataRed_heavy_edge_single_end() {
    int cost = 0;
    redo:
    for(int i : g->active_nodes){
        int max_weight = 0;
        int sum = 0;
        int i_max = -1;
        int j_max = -1;
        for(int j : g->active_nodes){
            if(i == j) continue;
            int weight = g->get_weight(i,j);
            if(weight > max_weight){
                max_weight = weight;
                i_max = i;
                j_max = j;
            }
            if(weight != DO_NOT_ADD && DO_NOT_DELETE)
                sum += abs(weight);
        }
        if(max_weight >= sum - max_weight && i_max != -1) {
            cost += g->merge(i_max, j_max);
            goto redo;
        }
    }
    if(cost > 0) printDebug("Merged vertices for dataRed_heavy_edge_single_end, cost: " + std::to_string(cost));
    return cost;
}


//is doing the large Neighbourhood Rule for all vertices in the graph TODO
int Solver::dataRed_large_neighbourhood_I() {
    return 0;
}

// param: u is the index of the vertex of which the neighbours are collected
// return: - pair for neighbourhood(first item in pair) - all the vertices that are adjacent to u,
//         - not_neighbours(second item in pair) - all vertices that are not adjacent to u
//
std::pair<std::list<int>, std::list<int>> Solver::closed_neighbourhood(int u) {
    std::list<int> neighbours;
    std::list<int> not_neighbours;
    for(int i : g->active_nodes){
        if(g->get_weight(u,i) > 0){
            neighbours.push_back(i);
        }else if(g->get_weight(u,i) < 0){
            not_neighbours.push_back(i);
        }
    }
    neighbours.push_back(u);
    return std::pair<std::list<int>, std::list<int>>(neighbours, not_neighbours);
}

//calculates the costs to make the neighbourhood a clique
int Solver::deficiency(std::list<int> neighbours) {
    int costs = 0;
    while (!neighbours.empty()) {
        int i = neighbours.front();
        neighbours.pop_front();
        for (int j : neighbours) {
            if (g->get_weight(i, j) < 0) {
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
            int weight = g->get_weight(i,j);
            if(weight > 0){
                cut_costs += weight;
            }
        }
    }
    return cut_costs;
}
// ----------------------------
// ------- merging --------

// unmerges all remaining vertices (after the solver finished) and outputs all modified edges
void Solver::final_unmerge_and_output(){
    int k = 0;
    while (g->merge_map.size() != g->num_vertices){
        verify_clusterGraph();
        int uv = g->merge_map.size() - 1;
        int dk = unmerge_and_output(uv);
        k_backward.insert(k_backward.begin(), dk);
        k += dk;
    }
    printDebug("\nUnmerging sum of costs " +  std::to_string(k));

}

// unmerges vertex uv and outputs all modified edges (with u and v) to keep same edge connection as with uv
int Solver::unmerge_and_output(int uv){
    std::vector<int> uv_children = g->merge_map[uv];
    int u = uv_children[0];
    int v = uv_children[1];

    printDebug("Output for unmerging (" + std::to_string(u) + "," + std::to_string(v) + ") -> " + std::to_string(uv));

    int dk = 0;
    for(int x : g->active_nodes) {
        if(x == uv || x == u ||x == v) continue;

        // NOTE: if w(u,x) == 0 we have to do something ----> delete or add depending on w(uv,x)
        if (g->get_weight(u, x) == 0 || (signbit(g->get_weight(u, x)) != signbit(g->get_weight(uv, x)))) {
            dk += abs(g->get_weight(u,x));

            // if w(uv,x) == 0 it doesnt matter if we delete or add ---> add edge
            if(g->get_weight(uv, x) >= 0)
                g->add_edge(u,x);
            if(g->get_weight(uv, x) < 0)
                g->delete_edge(u,x);

            if(u < g->num_vertices && x < g->num_vertices)
                final_output(u,x);
            else
                printDebug("Output queue: " + std::to_string(u+1-1) + " " + std::to_string(x+1-1));
        }
        if (g->get_weight(v, x) == 0 ||(signbit(g->get_weight(v, x)) != signbit(g->get_weight(uv, x)))) {
            dk += abs(g->get_weight(v,x));
            if(g->get_weight(uv, x) >= 0)
                g->add_edge(v,x);
            if(g->get_weight(uv, x) < 0)
                g->delete_edge(v,x);

            if(v < g->num_vertices && x < g->num_vertices)
                final_output(v,x);
            else
                printDebug("Output queue: " + std::to_string(v+1-1) + " " + std::to_string(x+1-1));

        }
    }
    printDebug("Unmerged " +  std::to_string(uv) + " -> (" + std::to_string(u) + "," + std::to_string(v) + ")" + " with cost " + std::to_string(dk));

    g->unmerge(uv);

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
    auto p3 = this->get_max_cost_p3_naive();
    if(std::get<0>(p3) == -1){
        printDebug("\nVERIFICATION SUCCESS\n");
    } else {
        printDebug("\nVERIFICATION FAIL:");
        print_tuple(p3);
    }
}







