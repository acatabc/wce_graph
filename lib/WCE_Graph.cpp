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

void WCE_Graph::delete_edge(int, int) {

}

void WCE_Graph::find_p3() {

}

void WCE_Graph::solve() {

}

void WCE_Graph::print(std::ostream& os) {
    for(int i = 0; i < this->num_vertices; ++i){
        for(int j = 0; j < this->num_vertices; ++j){
            os << this->adj_matrix[i][j] << " ";
        }
        os << std::endl;
    }
}

std::tuple<int, int, int> WCE_Graph::find_next_p3() {
    static int i, j = 0;
    std::cout << i << " " << j << std::endl;
    for(; i < this->num_vertices; ++i){
        for(j = 0;j < this->num_vertices; ++j){
            if(this->adj_matrix[i][j] > 0){
                for(int k = j+1; k < this->num_vertices; ++k ){
                    if(this->adj_matrix[i][k] > 0 && this->adj_matrix[j][k] < 0){
                        return std::make_tuple(i ,k, j);
                    }
                }
            }
        }
    }
    i = 0;
    j = 0;
    return std::make_tuple(-1,-1, -1);
}
