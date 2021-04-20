//
// Created by acat on 16.04.21.
//

#include "WCE_Graph.h"
#include <iostream>
#include <tuple>

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



void WCE_Graph::add_edge(int v, int w, int weight) {
    this->adj_matrix[v][w] = weight;
    this->adj_matrix[w][v] = weight;
}

void WCE_Graph::modify_edge(int v, int w) {
    this->adj_matrix[v][w] *= -1;
    this->adj_matrix[w][v] *= -1;
#ifdef DEBUG
    //std::cout << "deleting " << v << " " << w << std::endl;
#endif
}

int WCE_Graph::get_weight(int v, int w) {
    return this->adj_matrix[v][w];
}

void WCE_Graph::printGraph(std::ostream& os) {
    os << "Adjacency Matrix\n";
    for(int i = 0; i < this->num_vertices; ++i){
        for(int j = 0; j < this->num_vertices; ++j){
            os << this->adj_matrix[i][j] << " ";
        }
        os << std::endl;
    }
}
