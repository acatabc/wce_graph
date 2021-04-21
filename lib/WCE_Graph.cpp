//
// Created by acat on 16.04.21.
//

#include "WCE_Graph.h"
#include <iostream>
#include <tuple>
#include <cmath>
#include <iomanip>
#include <vector>
#include "./../include/utils.h"

#define NONE -1
#define CLUSTER_GRAPH -2
#define DO_NOT_DELETE INT32_MAX
#define DO_NOT_ADD 0

WCE_Graph::WCE_Graph(int n): num_vertices(n){
    this->adj_matrix = new int*[n];
    for(int i = 0; i < n ; ++i){
        adj_matrix[i]= new int[n];
        adj_matrix[i][i] = 0;
    }

}

WCE_Graph::~WCE_Graph() {
    for(int i = 0; i < num_vertices; ++i){
        delete[] adj_matrix[i];
    }
    delete[] adj_matrix;
}


void WCE_Graph::add_edge(int v, int w) {
    this->adj_matrix[v][w] = DO_NOT_DELETE;
    this->adj_matrix[w][v] = DO_NOT_DELETE;
}

void WCE_Graph::modify_edge(int v, int w) {
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


void WCE_Graph::print(std::ostream& os) {

    for(int i = 0; i <= this->num_vertices; ++i)
        if(i < 10)
            os << i <<std::setw(5)<< "|";
        else if(i < 100)
            os << i << std::setw(4) << "|";
        else
            os << i << std::setw(3) << "|";
    os << std::endl;
    for(int i = 0; i <= this->num_vertices; ++i){
        os << "-----+";
    }
    os << std::endl;
    for(int i = 0; i < this->num_vertices; ++i){
        if(i < 9) {
            os << i + 1 << std::setw(5) << "|";
        }else{
            os << i + 1 << std::setw(4) << "|";
        }
        for(int j = 0; j < this->num_vertices; ++j){
            int el = this->adj_matrix[i][j];
            if(el>= 0 && el <= 9)
                os << el << std::setw(5) << "|";
            else if(el >= 10 && el < 100 || el < 0 && el > -10)
                os << el << std::setw(4) << "|";
            else if(el == DO_NOT_DELETE)
                os << "DND" << std::setw(3) << "|";
            else if (el >= 100 && el < 1000 || el <= -10 && el > -100)
                os << el << std::setw(3) << "|";
            else
                os << el << std::setw(2) << "|";
        }
        os << std::endl;
    }
    os <<"\n\n"<< std::endl;
}
