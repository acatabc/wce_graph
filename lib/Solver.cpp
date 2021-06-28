
#include "Solver.h"
#include <tuple>
#include "iostream"
#include "../include/utils.h"
#include <math.h>

//const char* FILENAME = "../wce-students/specialTests/w27.dimacs";
const char* FILENAME = "../wce-students/2-real-world/w005.dimacs";
//const char* FILENAME = "../../wce-students-real/2-real-world/w061.dimacs";
//const char* FILENAME = "../test_data/w001.dimacs";

#define NONE (-1)
#define CLUSTER_GRAPH (-2)



Solver::Solver(){
    this->g = this->parse_and_build_graph();
}

Solver::~Solver() = default;

void Solver::solve() {

    int cost_before_branching = data_reduction_before_branching();
    int stack_before_branching = g->graph_mod_stack.size();

    // compute lower bound iterating over all existent p3s
    int lower_bound_k = std::get<1>(get_best_p3_and_lower_bound(MAX_SUM_P3, LOWER_BOUND_FAST));

    int k = lower_bound_k + cost_before_branching;

    while (true){
        printDebug("\nSOLVE FOR k:" + std::to_string(k));

        int k_reduced = k - cost_before_branching;

        k_reduced = data_reduction(k_reduced, 0);

        if(this->branch(k_reduced, 0) == CLUSTER_GRAPH){
            clear_stack_and_output();
            break;
        }
        g->recover_graph(stack_before_branching);
        k++;
    }

    g->verify_cluster_graph(); // only used in debug

    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(k) + "\n");
}


int Solver::branch(int k, int layer){
    if(k < 0){
        return NONE;
    }

    // data reduction
    int stack_size_0 = g->graph_mod_stack.size(); // save stack size to recover current graph after data reduction
    k = data_reduction(k, layer);
    if(k == -1){ // data reduction shows that no solution for this k exists
        g->recover_graph(stack_size_0);
        printDebug("=== fail layer " + std::to_string(layer) + " (data red)");
        return NONE;
    }

    // get best p3 and compute a lower bound
    std::tuple<Solver::p3, int> tuple;
    tuple = get_best_p3_and_lower_bound(MAX_SUM_P3, LOWER_BOUND_FAST);
    auto p3 = std::get<0>(tuple);
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
//        final_output(u,v);
        return CLUSTER_GRAPH;
    }
    else g->recover_graph(stack_size_1);

    // 2. Branch MERGE (u,v)
    // since deleting failed any solution must contain (u,v)
    // after merging (u,v) the p3 (u,v,w) will be resolved
    printDebug("Branch MERGE (" + std::to_string(u) + "," + std::to_string(v) + ") -> " + std::to_string(g->merge_map.size()) );
    int cost = g->merge(u,v);
    k -= cost;
    if(cost == -1) {  // both (delete/merge) failed -> no solution for this k exists
        g->recover_graph(stack_size_0);
        printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");
        return NONE;
    }
    if(this->branch(k, layer + 1) == CLUSTER_GRAPH){
        // nothing to output here
        return CLUSTER_GRAPH;
    }

    // both branches (delete/merge) failed, no solution for this k exists, recover graph before branching
    g->recover_graph(stack_size_0);
    printDebug("=== fail layer " + std::to_string(layer) + " with P3 (" + std::to_string(u) + "," + std::to_string(v) + ","+ std::to_string(w) + ")");
    return NONE;
}



// ----------------------------
// ------- output related --------


// unmerges all remaining vertices and outputs edges that had to be modified for merging
void Solver::clear_stack_and_output(){
    int k = 0;
    while (g->graph_mod_stack.size() != 0){
        WCE_Graph::stack_elem el = g->graph_mod_stack.top();
        if(el.type == MERGE) {
            printDebug("unmerge " + std::to_string(el.uv));
            if(el.v1 < g->num_vertices && el.v2 < g->num_vertices){
                if(g->get_weight(el.v1, el.v2) <= 0){
                    final_output(el.v1, el.v2);
                }
            }
            g->add_edge(el.v1, el.v2);
            k += unmerge_and_propagate(el.uv);

        }
        else if(el.type == SET_INF) {
            g->graph_mod_stack.pop();
            if(el.v1 < g->num_vertices && el.v2 < g->num_vertices){
                if(g->get_weight_original(el.v1, el.v2) > 0){
                    final_output(el.v1, el.v2);
                }
            }
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
int Solver::unmerge_and_propagate(int uv){
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

    g->unmerge(uv);

    printDebug("Unmerged " +  std::to_string(uv) + " -> (" + std::to_string(uv_children[0]) + "," + std::to_string(uv_children[1]) + ")" + " with cost " + std::to_string(dk));

    return dk;
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

WCE_Graph *Solver::parse_and_build_graph(){

#ifdef DEBUG
    //    freopen("../wce-students/2-real-world/w027.dimacs", "r", stdin);
//    freopen("../test_data/r049.dimacs", "r", stdin);
    auto x = freopen(FILENAME, "r", stdin);
    if (!x) printDebug("Cannot open file");
#endif
    int num_vertices = 0;
    std::cin >> num_vertices;

    if(num_vertices > MAX_NUM_VERTICES){  // greedy-greedy: output all positive edges if too many vertices in input graph
        int v, w, weight;
        while(std::cin) {
            std::cin >> v >> w >> weight;
            if(weight > 0) std::cout << v << " " << w << "\n";
        }
        return new WCE_Graph(num_vertices);
    }

    WCE_Graph *g = new WCE_Graph(num_vertices);

    int v, w, weight;
    while(std::cin){
        std::cin >> v >> w >> weight;
        v -= 1;
        w -= 1;
        if(!std::cin.fail()) {
            g->set_weight(v, w, weight);
            g->set_weight_original(v, w, weight);
        }
    }
    return g;
}




// ------ Debug ----

