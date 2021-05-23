
#include "WCE_Graph.h"
#include <iostream>
#include <tuple>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include "../include/utils.h"

WCE_Graph::WCE_Graph(int n): num_vertices(n){
    for(int i = 0; i < n ; i++){
        this->adj_matrix.push_back(std::vector<matrix_entry>(i));
        for(int j = 0; j < i; j++){
            matrix_entry entry = {0, true};
            adj_matrix[i][j] = entry;
        }
    }
}

WCE_Graph::~WCE_Graph() {
    adj_matrix.clear();
}

void WCE_Graph::set_weight(int v, int w, int weight){
    if(v == w){
        throwError( "set_weight error: u and w have same value " );
        return;
    }
    if(v > w){
        if(this->adj_matrix[v][w].flag == false){
            throwError( "set_weight error: invalid flag" );
            return;
        } else{
            this->adj_matrix[v][w].weight = weight;
            return;
        }
    }
    if(w > v) {
        if (this->adj_matrix[w][v].flag == false) {
            throwError( "set_weight error: invalid flag" );
            return;
        } else {
            this->adj_matrix[w][v].weight = weight;
            return;
        }
    }
    return;
}

int WCE_Graph::get_weight(int v, int w){
    if(v == w){
        throwError( "get_weight error: u and w have same value edge (" + std::to_string(v) + "," + std::to_string(w) + ")" );
        return 0;
    }

    if(v > w){
        if(this->adj_matrix[v][w].flag == false){
            throwError( "get_weight error: invalid flag edge (" + std::to_string(v) + "," + std::to_string(w) + ")" );
            return 0;
        } else{
            return this->adj_matrix[v][w].weight;
        }
    }
    if(w > v) {
        if (this->adj_matrix[w][v].flag == false) {
            throwError( "get_weight error: invalid flag edge (" + std::to_string(v) + "," + std::to_string(w) + ")" );
            return 0;
        } else {
            return this->adj_matrix[w][v].weight;
        }
    }
    return 0;
}

void WCE_Graph::add_edge(int v, int w) {
    set_weight(v,w, DO_NOT_DELETE);
}

void WCE_Graph::delete_edge(int v, int w) {
    set_weight(v,w, DO_NOT_ADD);
}

void WCE_Graph::modify_edge(int v, int w) {
    int vw = get_weight(v,w);
    set_weight(v,w, -vw);
}

// returns sum of absolute value of edge cost of p3 (u,v,w)
int WCE_Graph::get_cost(int u, int v, int w) {
    int sum = 0;
    int uv = get_weight(u,v);
    int uw = get_weight(u,w);
    int wv = get_weight(w,v);
    if(uv != DO_NOT_DELETE && uv != DO_NOT_ADD)
        sum += abs(uv);
    if(uw != DO_NOT_DELETE && uw != DO_NOT_ADD)
        sum += abs(uw);
    if(wv != DO_NOT_DELETE && wv != DO_NOT_ADD)
        sum += abs(wv);
    return sum;
}


// ---------------------------------------
// ---------- merging related ------------

// merges vertices u and v and returns costs for merging
// adds merging to modification stack
// returns -1 if merging is not possible because of conflicting edges (DO_NOT_DELETE/DO_NOT_ADD)
int WCE_Graph::merge(int u, int v) {

    if(get_weight(u,v) == DO_NOT_ADD) throwError("Cannot merge edge which is DNA ");

    int idx = this->adj_matrix.size();

    int c = components_map[u];
    if(c != components_map[v]) throwError("Trying to merge vertiices from different components");

    graph_mod_stack.push(stack_elem{.type = MERGE, .v1 = -1, .v2 = -1, .weight = -1, .uv = idx});

    // setup adjacency matrix for new vertex
    this->adj_matrix.push_back(std::vector<matrix_entry>());
    for(int i = 0; i < this->adj_matrix.size(); i++){
        this->adj_matrix[idx].push_back(matrix_entry{0,false});
    }

    // update merge_map and components map
    std::vector<int> uv = {u,v};
    this->merge_map.push_back(uv);

    // add active node uw to component c and remove u and v
    this->components_map.push_back(c);
    this->active_nodes[c].push_back(idx);
    std::vector<int>::iterator it;
    for(it=active_nodes[c].begin(); it != active_nodes[c].end(); ++it){
        if(*it == u || *it == v){
            active_nodes[c].erase(it);
            it -=1;
        }
    }

    int dk = 0;

    // compute weights (uv,j) based on (u,j) and (v,j) for all vertices j in the same component
    for(int j : active_nodes[c]){
        if(j == idx) continue;

        int weight_uj = this->get_weight(u, j);
        int weight_vj = this->get_weight(v, j);

        // if one edge is DO_NOT_ADD and the other one is DO_NOT_DELETE, this merging fails
        if((weight_uj == DO_NOT_ADD && weight_vj == DO_NOT_DELETE ) ||(weight_vj == DO_NOT_ADD && weight_uj == DO_NOT_DELETE)){
            this->unmerge(idx);
            printDebug("Merging (" + std::to_string(u) + "," + std::to_string(v) + ") failed ");
            return -1;
        }
            // if at least one of the two edges is DO_NOT_ADD/DO_NOT_DELETE, the merged one becomes DO_NOT_ADD/DO_NOT_DELETE as well
        else if (weight_uj == DO_NOT_ADD || weight_vj == DO_NOT_ADD)
            adj_matrix[idx][j].weight = DO_NOT_ADD;
        else if (weight_uj == DO_NOT_DELETE || weight_vj == DO_NOT_DELETE)
            adj_matrix[idx][j].weight = DO_NOT_DELETE;
            // otherwise use basic sum of edge cost
        else adj_matrix[idx][j].weight = this->get_weight(u, j) + this->get_weight(v, j);

        adj_matrix[idx][j].flag = true;

        // if the sign of the edges differ we have to add the hidden merging cost
        if((weight_vj > 0 &&  weight_uj < 0 ) || (weight_vj < 0 && weight_uj > 0 )){
//            printDebug("weights vj: " + std::to_string(abs(weight_vj)) + "  uj: "+ std::to_string(abs(weight_uj)) + "    dk: " + std::to_string(dk));
            // at most one of vj and uj can be DO_NOT_DELETE or DO_NOT_ADD -> most cases covered by min(..)
            if(weight_uj == DO_NOT_ADD || weight_vj == DO_NOT_ADD)
                // prevent overflow which is happening in abs(-inf)
                dk += std::max(weight_vj, weight_uj);
            else
                dk += std::min(abs(weight_vj), abs(weight_uj));
        }
    }


    printDebug("Merging (" + std::to_string(u) + "," + std::to_string(v) + ") -> " +  std::to_string(idx) + "     with cost " + std::to_string(dk));

    return dk;
}

// unmerges vertex uv and removes uf from mofication stack
// throws error if uv is not on top of modification stack
void WCE_Graph::unmerge(int uv) {
    // verify that this vertex is on top of modification stack
    stack_elem top = graph_mod_stack.top();
    if(top.type != MERGE && top.uv != uv){
        throwError("Error: unmerges vertex which is not on top of stack");
        return;
    }
    else graph_mod_stack.pop();

    // verify that uv is a merged vertex
    if(this->merge_map[uv].size() == 1) {
        throwError("Error: uv is not a merged vertex");
        return;
    }

    std::vector<int> uv_nodes = this->merge_map[uv];
    int u = uv_nodes[0];
    int v = uv_nodes[1];

    int c = components_map[uv];

    // remove uv from active nodes
    std::vector<int>::iterator it;
    for(it=active_nodes[c].begin(); it != active_nodes[c].end(); ++it){
        if(*it == uv){
            active_nodes[c].erase(it);
            it -=1;
        }
        if(*it > uv)
            *it -= 1;
    }

    // add u,v to active nodes
    this->active_nodes[c].push_back(u);
    this->active_nodes[c].push_back(v);

    // remove uw from merge map, components map and adjacency matrix
    merge_map.pop_back();
    components_map.pop_back();
    adj_matrix.pop_back();

//    printDebug("Unmerged " +  std::to_string(uv) + " -> (" + std::to_string(u) + "," + std::to_string(v) + ")");
    std::sort(active_nodes[c].begin(), active_nodes[c].end());
    return;
}

// set edge {u,v} = -inf and adds operation to modification stack
void WCE_Graph::set_non_edge(int u, int v) {
    graph_mod_stack.push(stack_elem{2, u, v, this->get_weight(u, v), -1});
    set_weight(u, v, DO_NOT_ADD);
//    printDebug("Heavy non edge (" + std::to_string(u) + "," + std::to_string(v) + ")");
}


void WCE_Graph::split_component(std::vector<std::vector<int>> components){
    int stack_size_before = graph_mod_stack.size();

    // delete all 0-edged between components
    for(int i = 0; i < components.size(); i++) {
        for(int u: components[i]){
            for(int j = i+1; j < components.size(); j++) {
                for(int v: components[j]){
                    if(get_weight(u,v) == 0) set_non_edge(u,v);
                }
            }
        }
    }

    int old_c = components_map[components[0][0]];
    std::vector<int> new_component_indices = std::vector<int>(); // vector of new components indices for graph_mod_stack

    // remove all elements from old component; add all elements of the first component
    active_nodes[old_c].clear();
    for(int u: components[0]){
        active_nodes[old_c].push_back(u);
        components_map[u] = old_c;
    }
    new_component_indices.push_back(old_c);

    for(int i = 1; i < components.size(); i++) {
        // add a new component and insert its vertices
        active_nodes.push_back(std::vector<int>());
        int new_c = active_nodes.size() - 1;
        for (int u: components[i]) {
            active_nodes[new_c].push_back(u);
            components_map[u] = new_c;
        }
        new_component_indices.push_back(new_c);
    }

    graph_mod_stack.push(
            WCE_Graph::stack_elem{.type = COMPONENTS, .v1 = -1, .v2 = -1, .weight = -1, .uv = -1, .components = new_component_indices, .stack_size_before_components = stack_size_before});

}


void WCE_Graph::unify_components(std::vector<int> component_indices,  int prev_stack_size){
    // put all nodes into the first component
    int comp_0 = component_indices[0];
    for(int c: component_indices){
        if(c == comp_0) continue;
        // copy nodes from all other component_indices to first component
        for(int u: active_nodes[c]){
            active_nodes[comp_0].push_back(u);
            components_map[u] = comp_0;
        }
    }

    // delete all resolved component_indices
    for(int i = 1; i < component_indices.size(); i++){
        active_nodes.pop_back(); // this works since we undo data reduction in the correct order
    }
    graph_mod_stack.pop();

    // recover all edges that have been set to DNA
    while (graph_mod_stack.size() != prev_stack_size){
        undo_final_modification();
    }

    printDebug("unified components");
}

void WCE_Graph::undo_final_modification(){
    stack_elem el = graph_mod_stack.top();
    if(el.type == MERGE)
        unmerge(el.uv);
    if(el.type == SET_INF){
        set_weight(el.v1, el.v2, el.weight);
        graph_mod_stack.pop();
//        printDebug("undo non-edge (" + std::to_string(el.v1) + ","+ std::to_string(el.v2) + ")" );
    }
    if(el.type == COMPONENTS){
         unify_components(el.components, el.stack_size_before_components);
    }
}


// ---------------------------------------
// ----------- print functions -----------

void WCE_Graph::print_merge_map() {
#ifdef DEBUG
    std::cout << "Merge Map: \n";
    for(int i = 0; i < this->merge_map.size(); ++i){
        std::cout << i << " --> " ;
        if(this->merge_map[i].size() == 1)
            std::cout << merge_map[i][0] << " \n" ;
        else
            std::cout << "(" << merge_map[i][0] << "," << merge_map[i][1] <<") \n" ;
    }
#endif
}

void WCE_Graph::print_active_nodes() {
#ifdef DEBUG
    std::cout << "Active Nodes Vec:\n";
    for(int c = 0; c < active_nodes.size(); c++) {
        std::cout << "Component " << c << ": [";
        for (int i = 0; i < this->active_nodes[c].size(); ++i) {
            if (i != this->active_nodes[c].size() - 1)
                std::cout << active_nodes[c][i] << ", ";
            else
                std::cout << active_nodes[c][i] << "]\n";
        }
    }
#endif
}


void WCE_Graph::print_graph_mod_stack()
{
    printDebug("-------");
    printDebug("Graph Modification stack: ");

    print_graph_mod_stack_rec();

    printDebug("-------");

}

void WCE_Graph::print_graph_mod_stack_rec()
{
    if (graph_mod_stack.empty()) return;

    WCE_Graph::stack_elem el = graph_mod_stack.top();

    graph_mod_stack.pop();

    print_graph_mod_stack_rec();

    if(el.type == MERGE) printDebug("Merged: " + std::to_string(el.uv));
    if(el.type == SET_INF) printDebug("SET_INF: " + std::to_string(el.v1) + "," + std::to_string(el.v2) );
    if(el.type == COMPONENTS) {
        printDebug("COMPONENT");
        printVector_int(el.components);
    }
    graph_mod_stack.push(el);
}


void WCE_Graph::print_active_graph(std::ostream& os) {
#ifdef DEBUG

    std::cout << "\nActive Graph: \n";

    for(int c = 0; c < active_nodes.size(); c++) {

        std::cout << "Component " << c << ":\n";

        sort(active_nodes[c].begin(), active_nodes[c].end());

        os << "." << std::setw(5) << "|";

        for (int j : active_nodes[c]) {
            if (j < 10)
                os << j << std::setw(5) << "|";
            else if (j < 100)
                os << j << std::setw(4) << "|";
            else
                os << j << std::setw(3) << "|";
        }
        os << std::endl;
        for (int i = 0; i <= active_nodes[c].size(); ++i) {
            os << "-----+";
        }
        os << std::endl;
        for (int i : active_nodes[c]) {
            if (i <= 9) {
                os << i << std::setw(5) << "|";
            } else {
                os << i << std::setw(4) << "|";
            }
            for (int j : active_nodes[c]) {

                if (i == j) {
                    os << "-" << std::setw(5) << "|";
                    continue;
                }

                bool flag;
                if (i < j) flag = this->adj_matrix[j][i].flag;
                if (i > j) flag = this->adj_matrix[i][j].flag;
                if (flag == false) {
                    os << " " << std::setw(5) << "|";
                    continue;
                }

                int el;
                if (i < j) el = this->adj_matrix[j][i].weight;
                if (i > j) el = this->adj_matrix[i][j].weight;

                if (el >= 0 && el <= 9)
                    os << el << std::setw(5) << "|";
                else if (el >= 10 && el < 100 || el < 0 && el > -10)
                    os << el << std::setw(4) << "|";
                else if (el == DO_NOT_DELETE)
                    os << "+DND" << std::setw(2) << "|";
                else if (el == DO_NOT_ADD)
                    os << "-DNA" << std::setw(2) << "|";
                else if (el >= 100 && el < 1000 || el <= -10 && el > -100)
                    os << el << std::setw(3) << "|";
                else
                    os << el << std::setw(2) << "|";
            }

            if (this->merge_map[i].size() != 1)
                os << " --> (" << merge_map[i][0] << "," << merge_map[i][1] << ")";
            os << std::endl;
        }
    }
    os <<"\n"<< std::endl;
#endif
}

void WCE_Graph::printGraph(std::ostream& os) {
#ifdef DEBUG
    std::cout << "\nTotal Graph: \n";

    int n_vertices = this->adj_matrix.size();

    for(int i = -1; i <= n_vertices-1; ++i)
        if(i == -1)
            os << i <<std::setw(4)<< "|";
        else if(i < 10)
            os << i <<std::setw(5)<< "|";
        else if(i < 100)
            os << i << std::setw(4) << "|";
        else
            os << i << std::setw(3) << "|";
    os << std::endl;
    for(int i = 0; i <= n_vertices; ++i){
        os << "-----+";
    }
    os << std::endl;
    for(int i = 0; i < n_vertices; ++i){
        if(i <= 9) {
            os << i << std::setw(5) << "|";
        }else{
            os << i << std::setw(4) << "|";
        }
        for(int j = 0; j < n_vertices; ++j){
            if(i==j) {
                os << " " << std::setw(5) << "|";
                continue;
            }

            bool flag;
            if(i<j) flag = this->adj_matrix[j][i].flag;
            if(i>j) flag = this->adj_matrix[i][j].flag;
            if(flag == false){
                os << " " << std::setw(5) << "|";
                continue;
            }

            int el;
            if(i<j) el = this->adj_matrix[j][i].weight;
            if(i>j) el = this->adj_matrix[i][j].weight;
            if(el>= 0 && el <= 9)
                os << el << std::setw(5) << "|";
            else if(el >= 10 && el < 100 || el < 0 && el > -10)
                os << el << std::setw(4) << "|";
            else if(el == DO_NOT_DELETE)
                os << "+DND" << std::setw(2) << "|";
            else if(el == DO_NOT_ADD)
                os << "-DNA" << std::setw(2) << "|";
            else if (el >= 100 && el < 1000 || el <= -10 && el > -100)
                os << el << std::setw(3) << "|";
            else
                os << el << std::setw(2) << "|";
        }

        if(this->merge_map[i].size() != 1)
            os << " --> (" << merge_map[i][0] << "," << merge_map[i][1] <<")";
        os << std::endl;
    }
    os <<""<< std::endl;
#endif
}

