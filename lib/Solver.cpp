
#include "Solver.h"
#include <tuple>
#include "iostream"
#include "../include/utils.h"
#include <math.h>

const char* FILENAME = "../wce-students/2-real-world/w053.dimacs";
//const char* FILENAME = "../../wce-students-real/2-real-world/w021.dimacs";
//const char* FILENAME = "../test_data/w001.dimacs";

#define NONE -1
#define CLUSTER_GRAPH -2

Solver::Solver(){
    this->g = this->parse_and_build_graph();

}

Solver::~Solver() {}

void Solver::solve() {

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

    int cost_before_branching = INT32_MAX - k_tmp;
    int stack_before_branching = g->graph_mod_stack.size();

    int k = 56;
    while (true){
        printDebug("\nSOLVE FOR k:" + std::to_string(k));


        int k_reduced = k - cost_before_branching;

        k_reduced = data_reduction(k_reduced, 0);

        g->print_graph_mod_stack();
        g->printGraph(std::cout);
        g->print_active_graph(std::cout);

        if(this->branch(k_reduced, 0) == CLUSTER_GRAPH){
            clear_stack_and_output();
            break;
        }
        undo_data_reduction(stack_before_branching);
        k++;
    }

    verify_clusterGraph(); // only used in debug

    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(k) + "\n");
}



int Solver::branch(int k, int layer){
    if(k < 0){
        return NONE;
    }

    g->print_active_graph(std::cout);

    // data reduction
    int stack_size = g->graph_mod_stack.size(); // save stack size to recover current graph after data reduction
    k = data_reduction(k, layer);
    if(k == -1){ // data reduction shows that no solution for this k exists
        undo_data_reduction(stack_size);
        printDebug("=== fail layer " + std::to_string(layer) + " (data red)");
        return NONE;
    }

    auto p3 = this->get_max_cost_p3_naive();

    if(std::get<0>(p3) == -1){
        printDebug("FOUND CLUSTER GRAPH");
        return CLUSTER_GRAPH;
    }

    rec_steps++;

    int u = std::get<0>(p3);
    int v = std::get<1>(p3);
    int w = std::get<2>(p3);

    printDebug("Layer " + std::to_string(layer) + " P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");


    // 1. branch on (u,v) >= 0
    int weight_uv = g->get_weight(u,v);

    if(weight_uv < 0) throwError("WHY IS THIS smaller 0?");
    if(weight_uv == DO_NOT_DELETE) throwError("WHY IS THIS DND?");

    printDebug("Branch DELETE (" + std::to_string(u) + "," + std::to_string(v) + ")");
    g->delete_edge(u, v);
    printDebug(std::to_string(g->graph_mod_stack.size()));
    if(this->branch(k - weight_uv, layer + 1) == CLUSTER_GRAPH){
        final_output(u,v);
        return CLUSTER_GRAPH;
    }

    // if deleting failed, any solution must contain (u,v) -> merge
    g->add_edge(u, v);
    int cost = g->merge(u,v);
    if(cost == -1) {  // also inserting (u,v) failed -> no solution for this k exists
        undo_data_reduction(stack_size);
        g->set_weight(u,v, weight_uv);
        printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");
        return NONE;
    }
    k -= cost;
    int uv = g->merge_map.size()-1;
    printDebug("Branch MERGE (" + std::to_string(u) + "," + std::to_string(v) + ") -> " + std::to_string(uv) );



    // 2. branch on edge (uv,w)
    int weight_uv_w = g->get_weight(uv,w);

    if(weight_uv_w == DO_NOT_DELETE) throwError("WHY IS THIS DND?");

    // first try deleting (uv,w)
    printDebug("Branch DELETE (" + std::to_string(uv) + "," + std::to_string(w) + ")");
    g->delete_edge(uv, w);
    printDebug(std::to_string(g->graph_mod_stack.size()));
    int deleting_costs = std::max(0,weight_uv_w); // if (uv,w) < 0 there is no cost for deletion
    if(this->branch(k - deleting_costs, layer + 1) == CLUSTER_GRAPH){
        final_output(uv,w);
        return CLUSTER_GRAPH;
    }

    // if deleting failed, any solution must contain (uv,w) -> merge
    if(weight_uv_w == DO_NOT_ADD){
        // we are not allowed to insert (uv,w) -> no solution for this k exists
        undo_data_reduction(stack_size);
        g->set_weight(u,v, weight_uv);
        printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");
        return NONE;
    }
    g->add_edge(uv,w);
    if(weight_uv_w < 0) k -= abs(weight_uv_w); // remove cost of inserting edge (uv,w)
    cost = g->merge(uv,w);
    if(cost == -1) { // also inserting (uv,w) failed -> no solution for this k exists
        undo_data_reduction(stack_size);
        g->set_weight(u,v, weight_uv);
        printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");
        return NONE;
    }
    k -= cost;
    printDebug("Branch MERGE (" + std::to_string(uv) + "," + std::to_string(w) + ")" );
    // last try with the merged vertices (uv,w)
    if(this->branch(k, layer + 1) == CLUSTER_GRAPH){
        final_output(uv,w);
        return CLUSTER_GRAPH;
    }

    // branching on all edges failed, no solution for this k exists, recover graph before branching
    undo_data_reduction(stack_size);
    g->set_weight(u,v, weight_uv);
    printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");
    return NONE;
}


void Solver::final_output(int u, int v)
{
    if(u < g->num_vertices && v < g->num_vertices){
        std::cout << u+1 << " " << v+1 <<std::endl;
        #ifdef DEBUG
                if(PRINTDEBUG){
//                std::cout << u+1-1 << " " << v+1-1 <<std::endl;
            }
        #endif
    }
    else{
        printDebug("output (later): " + std::to_string(u+1-1) + " " + std::to_string(v+1-1));
    }

}


// ----------------------------
// ------- p3 - search --------

// iterates over all vertex tuples and returns max_cost p3 = (u,v,w)
// (u,v) and (u,w) exist and (v,w) does not exist
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
// ------- merging --------


// unmerges all remaining vertices and outputs edges that had to be modified for merging
void Solver::clear_stack_and_output(){
    int k = 0;
    while (g->graph_mod_stack.size() != 0){
        WCE_Graph::stack_elem el = g->graph_mod_stack.top();
        if(el.type == MERGE) {
            int uv = el.uv;
            printDebug("unmerge " + std::to_string(uv));
            int dk = unmerge_and_output(uv);
            k += dk;
        }
        else if(el.type == SET_INF) {
            g->graph_mod_stack.pop();
        }
        else if(el.type == CLIQUE){
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

            printDebug("Propagate : " + std::to_string(uv) + " " + std::to_string(x) + " weight " + std::to_string(weight_uvx) + "    to    " + std::to_string(u) + " " + std::to_string(x) + " weight " + std::to_string(weight_ux) );
            if((weight_ux == DO_NOT_DELETE && weight_uvx == DO_NOT_ADD) || (weight_ux == DO_NOT_ADD && weight_uvx == DO_NOT_DELETE) ) throwError("Cannot propagate, would need infinite budget");

            // adapt edge {u,x} to previous merged edge {uv,x}
            // we have to modify edge {u,x} if {u,x} = 0 and if {u,x} and {uv,x} have different sign
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

