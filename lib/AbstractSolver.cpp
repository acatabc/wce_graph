#include "AbstractSolver.h"

#include <tuple>
#include "iostream"
#include "../include/utils.h"
#include <math.h>



AbstractSolver::AbstractSolver(WCE_Graph *g):g(g){}

AbstractSolver::~AbstractSolver() = default;



// -------------------------------
// ------- output related --------


// unmerges all remaining vertices and outputs edges that had to be modified for merging
void AbstractSolver::clear_stack_and_output(){
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
int AbstractSolver::unmerge_and_propagate(int uv){
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

void AbstractSolver::final_output(int u, int v)
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




// ---------------------------
// ------- p3 related --------


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
