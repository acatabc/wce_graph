
#include "WCE_Graph.h"
#include <iostream>
#include <tuple>
#include <cmath>
#include <iomanip>
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
        std::cout << "set_weight error: u and w have same value \n";
        return;
    }
    if(v > w){
        if(this->adj_matrix[v][w].flag == false){
            std::cout << "set_weight error: invalid flag \n";
            return;
        } else{
            this->adj_matrix[v][w].weight = weight;
            return;
        }
    }
    if(w > v) {
        if (this->adj_matrix[w][v].flag == false) {
            std::cout << "set_weight error: invalid flag \n";
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
        std::cout << "get_weight error: u and w have same value \n";
        return 0;
    }

    if(v > w){
        if(this->adj_matrix[v][w].flag == false){
            std::cout << "get_weight error: invalid flag \n";
            return 0;
        } else{
            return this->adj_matrix[v][w].weight;
        }
    }
    if(w > v) {
        if (this->adj_matrix[w][v].flag == false) {
            std::cout << "get_weight error: invalid flag \n";
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
int WCE_Graph::merge(int u, int v) {

    int idx = this->adj_matrix.size();

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

        adj_matrix[idx][j].weight = this->get_weight(u, j) + this->get_weight(v, j);
        adj_matrix[idx][j].flag = true;

        if((weight_vj > 0 &&  weight_uj < 0 ) || (weight_vj < 0 && weight_uj > 0 )){
            dk += std::min(abs(weight_vj), abs(weight_uj));
        }
    }


    printDebug("Merging (" + std::to_string(u) + "," + std::to_string(v) + ") -> " +  std::to_string(idx) + "     with cost " + std::to_string(dk));

    return dk;
}

// unmerges vertex uv, take care to unmerge in correct order!
void WCE_Graph::unmerge(int uv) {

    // uv is not a merged vertex
    if(this->merge_map[uv].size() == 1) return;

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
    return;
}

// recovers original graph, merge map and actives nodes vector (as it has been before any merging operation)
void WCE_Graph::recover_original(){
    while (adj_matrix.size() != num_vertices){
        unmerge(adj_matrix.size()-1);
    }
    sort(active_nodes.begin(), active_nodes.end());
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
