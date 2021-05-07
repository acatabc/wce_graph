
#include "WCE_Graph.h"
#include <iostream>
#include <tuple>
#include <cmath>
#include <iomanip>
#include "../include/utils.h"


WCE_Graph::WCE_Graph(int n): num_vertices_original(n){
    this->adj_matrix = new int*[n];
    for(int i = 0; i < n ; ++i){
        adj_matrix[i]= new int[n];
        adj_matrix[i][i] = 0;
    }

}

WCE_Graph::~WCE_Graph() {
    for(int i = 0; i < num_vertices_original; ++i){
        delete[] adj_matrix[i];
    }
    delete[] adj_matrix;
}

void WCE_Graph::add_edge(int v, int w) {
    this->adj_matrix[v][w] = DO_NOT_DELETE;
    this->adj_matrix[w][v] = DO_NOT_DELETE;
}

void WCE_Graph::modify_edge(int v, int w) {
    this->adj_matrix[v][w] *= -1;
    this->adj_matrix[w][v] *= -1;
}

void WCE_Graph::delete_edge(int v, int w) {
    this->adj_matrix[v][w] = DO_NOT_ADD;
    this->adj_matrix[w][v] = DO_NOT_ADD;
#ifdef DEBUG
    //std::cout << "deleting " << v << " " << w << std::endl;
#endif
}

void WCE_Graph::set_weight(int v, int w, int weight){
    this->adj_matrix[v][w] = weight;
    this->adj_matrix[w][v] = weight;
}

int WCE_Graph::get_weight(int v, int w){
    return this->adj_matrix[v][w];
}



void WCE_Graph::printGraph(std::ostream& os) {
#ifdef DEBUG

    for(int i = -1; i <= this->num_vertices_original; ++i)
        if(i < 10)
            os << i <<std::setw(5)<< "|";
        else if(i < 100)
            os << i << std::setw(4) << "|";
        else
            os << i << std::setw(3) << "|";
    os << std::endl;
    for(int i = 0; i <= this->num_vertices_original; ++i){
        os << "-----+";
    }
    os << std::endl;
    for(int i = 0; i < this->num_vertices_original; ++i){
        if(i < 9) {
            os << i << std::setw(5) << "|";
        }else{
            os << i << std::setw(4) << "|";
        }
        for(int j = 0; j < this->num_vertices_original; ++j){
            int el = this->adj_matrix[i][j];
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
        os << std::endl;
    }
    os <<"\n\n"<< std::endl;
#endif
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
