//
// Created by acat on 16.04.21.
//

#include "WCE_Graph.h"
#include <iostream>
#include <tuple>
#define NONE -1
#define CLUSTER_GRAPH -2
#define RETURN_GRAPH -3
#define NAN

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

void WCE_Graph::delete_edge(int v, int w) {
    this->adj_matrix[v][w] *= -1;
    this->adj_matrix[w][v] *= -1;
#ifdef DEBUG
    std::cout << "deleting " << v << " " << w << std::endl;
#endif
}

int WCE_Graph::branch(int k){
    if(k < 0){
        return NONE;
    }
    auto p = this->find_first_p3();
    if(std::get<0>(p) == -1){
        return CLUSTER_GRAPH;
    }
    int v = std::get<0>(p);
    int w = std::get<1>(p);
    int u = std::get<2>(p);

    int weight = this->adj_matrix[v][w];
    this->delete_edge(v, w);

    this->print_all_p3();
    int ret_val = this->branch(k-weight);
    if(ret_val == CLUSTER_GRAPH){
        std::cout << v << " " << w <<std::endl;
        return CLUSTER_GRAPH;
    }
    this->delete_edge(v, w);

    weight = this->adj_matrix[v][u];
    this->delete_edge(v,u);
    ret_val = branch(k -weight);
    if(ret_val == CLUSTER_GRAPH){
        std::cout << v << " " << u <<std::endl;
        return CLUSTER_GRAPH;
    }
    this->delete_edge(v,u);

    weight = this->adj_matrix[w][u];
    this->delete_edge(w,u);
    ret_val = branch(k +weight);
    if(ret_val == CLUSTER_GRAPH){
        std::cout << w << " " << u <<std::endl;
        return CLUSTER_GRAPH;
    }
    this->delete_edge(w,u);
    return NONE;




}

void WCE_Graph::solve() {
    int k = 7;
    this->branch(k);
}

void WCE_Graph::print(std::ostream& os) {
    for(int i = 0; i < this->num_vertices; ++i){
        for(int j = 0; j < this->num_vertices; ++j){
            os << this->adj_matrix[i][j] << " ";
        }
        os << std::endl;
    }
}

std::tuple<int, int, int> WCE_Graph::find_first_p3() {
    //std::cout <<"startwerte "<< i << " " << j << std::endl;
    for(int i = 0; i < this->num_vertices; ++i){
        for(int j = 0;j < this->num_vertices; ++j) {
            //      std::cout << i << " "<< j << std::endl;
            if (this->adj_matrix[i][j] > 0) {
                for (int k = j + 1; k < this->num_vertices; ++k) {
                    if (this->adj_matrix[i][k] > 0 && this->adj_matrix[j][k] < 0) {
                        j++;
                        return std::make_tuple(i, k, j - 1);
                    }
                }
            }
        }
    }
    return std::make_tuple(-1,-1, -1);
}


std::tuple<int, int, int> WCE_Graph::find_next_p3() {
    static int i, j = 0;
    //std::cout <<"startwerte "<< i << " " << j << std::endl;
    for(; i < this->num_vertices; ++i){
        for(;j < this->num_vertices; ++j){
      //      std::cout << i << " "<< j << std::endl;
            if(this->adj_matrix[i][j] > 0){
                for(int k = j+1; k < this->num_vertices; ++k ){
                    if(this->adj_matrix[i][k] > 0 && this->adj_matrix[j][k] < 0){
                        j++;
                        return std::make_tuple(i ,k, j-1);
                    }
                }
            }
        }
        j = 0;
    }
    i = 0;
    j = 0;
    return std::make_tuple(-1,-1, -1);
}

void WCE_Graph::print_all_p3(){
    this->print(std::cout);
    std::cout << "++++++++++++++++++++++++" << std::endl;
    do{
        auto p = this->find_next_p3();
        std::cout << "(" << std::get<0>(p) <<", " << std::get<1>(p) << ", " << std::get<2>(p) << ")" << std::endl;
        if(std::get<0>(p) == -1)
            break;
    }while(1);

    std::cout << "++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
}
