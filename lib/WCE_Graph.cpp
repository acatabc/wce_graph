
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
            matrix_entry entry = {.weight = 0, .valid_entry = true, .weight_original = 0};
            adj_matrix[i][j] = entry;
        }

        std::vector<int> u = {i};
        merge_map.push_back(u);
        active_nodes.push_back(i);
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
        if(this->adj_matrix[v][w].valid_entry == false){
            throwError( "set_weight error: invalid valid_entry" );
            return;
        } else{
            this->adj_matrix[v][w].weight = weight;
            return;
        }
    }
    if(w > v) {
        if (this->adj_matrix[w][v].valid_entry == false) {
            throwError( "set_weight error: invalid valid_entry" );
            return;
        } else {
            this->adj_matrix[w][v].weight = weight;
            return;
        }
    }
    return;
}


void WCE_Graph::set_weight_original(int v, int w, int weight){
    if(v == w){
        throwError( "set_weight error: u and w have same value " );
        return;
    }
    if(v > w){
        if(this->adj_matrix[v][w].valid_entry == false){
            throwError( "set_weight error: invalid valid_entry" );
            return;
        } else{
            this->adj_matrix[v][w].weight_original = weight;
            return;
        }
    }
    if(w > v) {
        if (this->adj_matrix[w][v].valid_entry == false) {
            throwError( "set_weight error: invalid valid_entry" );
            return;
        } else {
            this->adj_matrix[w][v].weight_original = weight;
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
        if(this->adj_matrix[v][w].valid_entry == false){
            throwError( "get_weight error: invalid valid_entry edge (" + std::to_string(v) + "," + std::to_string(w) + ")" );
            return 0;
        } else{
            return this->adj_matrix[v][w].weight;
        }
    }
    if(w > v) {
        if (this->adj_matrix[w][v].valid_entry == false) {
            throwError( "get_weight error: invalid valid_entry edge (" + std::to_string(v) + "," + std::to_string(w) + ")" );
            return 0;
        } else {
            return this->adj_matrix[w][v].weight;
        }
    }
    return 0;
}


int WCE_Graph::get_weight_original(int v, int w){
    if(v == w){
        throwError( "get_weight error: u and w have same value edge (" + std::to_string(v) + "," + std::to_string(w) + ")" );
        return 0;
    }

    if(v > w){
        if(this->adj_matrix[v][w].valid_entry == false){
            throwError( "get_weight error: invalid valid_entry edge (" + std::to_string(v) + "," + std::to_string(w) + ")" );
            return 0;
        } else{
            return this->adj_matrix[v][w].weight_original;
        }
    }
    if(w > v) {
        if (this->adj_matrix[w][v].valid_entry == false) {
            throwError( "get_weight error: invalid valid_entry edge (" + std::to_string(v) + "," + std::to_string(w) + ")" );
            return 0;
        } else {
            return this->adj_matrix[w][v].weight_original;
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

    graph_mod_stack.push(stack_elem{.type = MERGE, .v1 = u, .v2 = v, .weight = -1, .uv = idx, .clique = std::vector<int>()});

    // setup adjacency matrix for new vertex
    this->adj_matrix.push_back(std::vector<matrix_entry>());
    for(int i = 0; i < this->adj_matrix.size(); i++){
        this->adj_matrix[idx].push_back(matrix_entry{0,false});
    }

    // update merge_map
    std::vector<int> uv = {u,v};
    this->merge_map.push_back(uv);

    // add active node uw, remove active nodes u and v
    this->active_nodes.push_back(idx);
    std::vector<int>::iterator it;
    for(it=active_nodes.begin(); it != active_nodes.end(); ++it){
        if(*it == u || *it == v){
            active_nodes.erase(it);
            it -=1;
        }
    }

    int dk = 0;

    // unmerging must happen in same order as merging!!!!
    for(int j : active_nodes){
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

        adj_matrix[idx][j].valid_entry = true;

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

    // remove uv from active nodes
    std::vector<int>::iterator it;
    for(it=active_nodes.begin(); it != active_nodes.end(); ++it){
        if(*it == uv){
            active_nodes.erase(it);
            it -=1;
        }
        if(*it > uv)
            *it -= 1;
    }

    // add u,v to active nodes
    this->active_nodes.push_back(u);
    this->active_nodes.push_back(v);

    // remove uw from merge map and adjacency matrix
    merge_map.pop_back();
    adj_matrix.pop_back();

//    printDebug("Unmerged " +  std::to_string(uv) + " -> (" + std::to_string(u) + "," + std::to_string(v) + ")");
    std::sort(active_nodes.begin(), active_nodes.end());
    return;
}

// set edge {u,v} = -inf and adds operation to modification stack
void WCE_Graph::set_non_edge(int u, int v) {
    graph_mod_stack.push(stack_elem{2, u, v, this->get_weight(u, v), -1});
    set_weight(u, v, DO_NOT_ADD);
//    printDebug("Heavy non edge (" + std::to_string(u) + "," + std::to_string(v) + ")");
}


void WCE_Graph::remove_clique(std::vector<int> &component){
    for(auto i = active_nodes.begin(); i != active_nodes.end(); ++i){
        for(int j : component){
            if(*i == j){
                i = active_nodes.erase(i);
                i--;
            }
        }
    }
    graph_mod_stack.push(WCE_Graph::stack_elem{.type = CLIQUE, .v1 = -1, .v2 = -1, .weight = -1, .uv = -1, .clique = component});
}


void WCE_Graph::recover_graph(int prev_stack_size){
    while (graph_mod_stack.size() != prev_stack_size){
        stack_elem el = graph_mod_stack.top();
        if(el.type == MERGE)
            unmerge(el.uv);
        if(el.type == SET_INF){
            set_weight(el.v1, el.v2, el.weight);
            graph_mod_stack.pop();
//        printDebug("undo non-edge (" + std::to_string(el.v1) + ","+ std::to_string(el.v2) + ")" );
        }
        if(el.type == CLIQUE){
            for(int i: el.clique){
                active_nodes.push_back(i);
            }
            graph_mod_stack.pop();
        }
    }
}


void WCE_Graph::reset_graph(){
    recover_graph(0);
    for(int i = 0; i < this->adj_matrix.size(); ++i){
        for(int j = i+1; j < this->adj_matrix.size(); ++j){
            set_weight(i,j, get_weight_original(i,j));
        }
    }
}


void WCE_Graph::DFS(int i, bool *visited, std::vector<int>& component) {
    visited[i] = true;
//    std::cout << i+1 << " ";
    component.push_back(i);
    for(int j : active_nodes){
        if(i == j) continue;
        // for {i,j} = 0 we consider i,j as not adjacent (we delete {i,j} later in unmerging)
        // thus, i and j are in different components if {i,j} <= 0
        if(get_weight(i,j) > 0){
            if(visited[j] == false){
                DFS(j, visited, component);
            }
        }
    }
    return;
}


// param: u is the index of the vertex of which the neighbours are collected
// return: - pair for neighbourhood(first item in pair) - all the vertices that are adjacent to u,
//         - not_neighbours(second item in pair) - all vertices that are not adjacent to u
//
std::pair<std::list<int>, std::list<int>> WCE_Graph::closed_neighbourhood(int u) {
    std::list<int> neighbours;
    std::list<int> not_neighbours;
    for(int i : active_nodes){
        if(i == u) continue;
        if(get_weight(u,i) > 0){
            neighbours.push_back(i);
        }else if(get_weight(u,i) <= 0){  // 0-edge = non-edge
            not_neighbours.push_back(i);
        }
    }
    neighbours.push_back(u);
    return std::make_pair(neighbours, not_neighbours);
}

//calculates the costs to make the neighbourhood a clique
int WCE_Graph::deficiency(std::list<int> neighbours) {
    int costs = 0;
    while (!neighbours.empty()) {
        int i = neighbours.front();
        neighbours.pop_front();
        for (int j : neighbours) {
            if ( i == j) continue;
            if (get_weight(i, j) < 0) {
                if(get_weight(i,j) == DO_NOT_ADD){
                    return DO_NOT_DELETE; // abs(DO_NOT_ADD) is DO_NOT_ADD again but a high value should be returned
                }
                costs += abs(get_weight(i, j));
            }
        }
    }
    return costs;
}

//calculates the cost to cut of the neighbourhood(neighbourhood) from the rest of the graph(rest_graph)
int WCE_Graph::cut_weight(std::list<int>& neighbourhood, std::list<int>& rest_graph) {
    int cut_costs = 0;
    for(int i : neighbourhood){
        for(int j : rest_graph){
            if(i == j)continue;
            int weight = get_weight(i,j);
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
    std::cout << "Active Nodes Vec: [";
    for(int i = 0; i < this->active_nodes.size(); ++i){
        if(i != this->active_nodes.size()-1)
            std::cout << active_nodes[i] << ", " ;
        else
            std::cout << active_nodes[i] << "]\n";
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
    if(el.type == CLIQUE) printDebug("CLIQUE " + std::to_string(el.clique.size()) );

    graph_mod_stack.push(el);
}


void WCE_Graph::print_active_graph(std::ostream& os) {
#ifdef DEBUG

    std::cout << "\nActive Graph: \n";

    sort(active_nodes.begin(), active_nodes.end());

    os << "." <<std::setw(5)<< "|";

    for(int j : active_nodes) {
        if (j < 10)
            os << j << std::setw(5) << "|";
        else if (j < 100)
            os << j << std::setw(4) << "|";
        else
            os << j << std::setw(3) << "|";
    }
    os << std::endl;
    for(int i = 0; i <= active_nodes.size(); ++i){
        os << "-----+";
    }
    os << std::endl;
    for(int i : active_nodes){
        if(i <= 9) {
            os << i << std::setw(5) << "|";
        }else{
            os << i << std::setw(4) << "|";
        }
        for(int j : active_nodes){

            if(i==j) {
                os << "-" << std::setw(5) << "|";
                continue;
            }

            bool flag;
            if(i<j) flag = this->adj_matrix[j][i].valid_entry;
            if(i>j) flag = this->adj_matrix[i][j].valid_entry;
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
            if(i<j) flag = this->adj_matrix[j][i].valid_entry;
            if(i>j) flag = this->adj_matrix[i][j].valid_entry;
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





// verify that the current graph is now a cluster graph
void WCE_Graph::verify_cluster_graph(){
#ifdef DEBUG
    printDebug("\nVerifying solution...");

    std::tuple<int, int, int> p3 = std::make_tuple(-1,-1,-1);
    int max_cost = INT32_MIN;
    for(int i: active_nodes){
        for(int j: active_nodes){
            for(int k : active_nodes){
                if(i == j || i == k || k == j) continue;
                int weight_ij = get_weight(i, j);
                int weight_ik = get_weight(i, k);
                int weight_jk = get_weight(j, k);
                if(weight_ij > 0 && weight_ik > 0 && weight_jk <= 0){
                    p3 = std::make_tuple(i,j,k);
                    goto end_p3_search;
                }
            }

        }
    }

    end_p3_search:
    if(std::get<0>(p3) == -1){
        printDebug("VERIFICATION SUCCESS\n");
    } else {
        printDebug("\nVERIFICATION FAIL:");
        print_tuple(p3);
        int u = std::get<0>(p3);
        int v = std::get<1>(p3);
        int w = std::get<2>(p3);
        std::cout << "(" << u << "," << v << "):" << get_weight(u,v) << "/" << get_weight_original(u,v) << "\n";
        std::cout << "(" << v << "," << w << "):" << get_weight(w,v) << "/" << get_weight_original(w,v) << "\n";
        std::cout << "(" << u << "," << w << "):" << get_weight(u,w) << "/" << get_weight_original(u,w) << "\n";
    }
#endif
}

