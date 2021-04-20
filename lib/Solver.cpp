//
// Created by Julia Henkel on 20.04.21.
//

#include "Solver.h"
#include "iostream"
#define NONE -1
#define CLUSTER_GRAPH -2

Solver::Solver(WCE_Graph *graph): g(graph){}

Solver::~Solver() {}

void Solver::solve() {
    printDebug("Solving...");

    int k = 0;
    printDebug(std::to_string(k) + "\n");
    while (this->branch(k) == NONE){
        printDebug(std::to_string(k) + "\n");
        k++;
    }
}

int Solver::branch(int k){
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

    if(this->branchEdge(u,v,k) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(v,w,k) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(w,u,k) == CLUSTER_GRAPH) return CLUSTER_GRAPH;

    return NONE;
}

int Solver::branchEdge(int u, int v, int k){
    int weight = g->get_weight(u,v);
    g->modify_edge(u, v);
    if(this->branch(k-weight) == CLUSTER_GRAPH){
        std::cout << u+1 << " " << v+1 <<std::endl;
        return CLUSTER_GRAPH;
    }
    g->modify_edge(u, v);
    return NONE;
}

std::tuple<int, int, int> Solver::find_first_p3() {
    for(int i = 0; i < g->num_vertices; ++i){
        for(int j = 0;j < g->num_vertices; ++j) {
            if (g->get_weight(i,j) > 0) {
                for (int k = j + 1; k < g->num_vertices; ++k) {
                    if (g->get_weight(i,k) > 0 && g->get_weight(j,k) < 0) {
                        return std::make_tuple(i, k, j);
                    }
                }
            }
        }
    }
    return std::make_tuple(-1,-1, -1);
}


std::tuple<int, int, int> Solver::find_next_p3() {
    static int i, j, k= 0;
    static int old_k = 0;
//    std::cout <<"startwerte "<< i << " " << j << std::endl;
    for(; i < g->num_vertices; ++i){
        for(;j < g->num_vertices; ++j){
            //      std::cout << i << " "<< j << std::endl;
            if(g->get_weight(i,j) > 0){

                for(; ; ){
//                    std::cout << k << std::endl;
                    if(k == 0)
                        k = j+1;
                    else {
                        k++;
                    }
                    if(k >= g->num_vertices) {
                        k = 0;
                        break;
                    }
                    if(g->get_weight(i,k) > 0 && g->get_weight(j,k) < 0){
                        return std::make_tuple(i ,k, j);
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

void Solver::print_all_p3(){
    printDebug("Find all p3");
    std::cout << "++++++++++++++++++++++++" << std::endl;
    do{
        auto p = this->find_next_p3();
        std::cout << "(" << std::get<0>(p) + 1 <<", " << std::get<1>(p) + 1 << ", " << std::get<2>(p) +1 << ")"<< std::endl;
        if(std::get<0>(p) == -1)
            break;
    }while(1);

    std::cout << "++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
}


void Solver::printDebug(std::string line){
#ifdef DEBUG
    std::cout << line << std::endl;
#endif
}
