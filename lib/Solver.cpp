//
// Created by Julia Henkel on 20.04.21.
//

#include <tuple>
#include "Solver.h"
#include "iostream"
#define NONE -1
#define CLUSTER_GRAPH -2
#define DO_NOT_DELETE INT32_MAX
#define DO_NOT_ADD 0

Solver::Solver(WCE_Graph *graph): g(graph){}

Solver::~Solver() {}

void Solver::solve() {
    printDebug("Solving...");

    int k = 0;
    while (this->branch(k) == NONE){
        k++;
        rec_steps = 0;
    }
    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(k) + "\n");
}

int Solver::branch(int k){
    rec_steps++;
    if(k < 0){
        return NONE;
    }
    int p3_weight;
    auto p = this->get_max_cost_p3(&p3_weight);


    int v = std::get<0>(p);
    int w = std::get<1>(p);
    int u = std::get<2>(p);

    if(std::get<0>(p) == -1){
        return CLUSTER_GRAPH;
    }

    if(this->branchEdge(u,v,k) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(v,w,k) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(w,u,k) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    rec_steps--;
    return NONE;
}

int Solver::branchEdge(int u, int v, int k){
    int weight = g->get_weight(u,v);
    if(weight > 0 && weight != DO_NOT_DELETE) {
        g->modify_edge(u, v);
    }else if(weight != DO_NOT_ADD){
        g->add_edge(u,v);
    }else{
        return NONE;
    }
    if(this->branch(k-abs(weight)) == CLUSTER_GRAPH){
        std::cout << u+1 << " " << v+1 <<std::endl;
        return CLUSTER_GRAPH;
    }
    g->set_weight(u, v, weight);

    return NONE;
}


std::tuple<int, int, int> Solver::find_next_p3() {
    static int i, j, k= 0;
    static int old_k = 0;
    for(; i < g->num_vertices; ++i){
        for(;j < g->num_vertices; ++j){
            if(g->get_weight(i,j) > 0){

                for(; ; ){
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
std::list<std::tuple<int, int, int>> Solver::get_all_p3() {
    std::list<std::tuple<int,int,int>> p3_list;
    for(int i = 0; i < this->g->num_vertices; ++i){
        for(int j = 0; j < this->g->num_vertices; ++j){
            if(this->g->get_weight(i,j) > 0){
                for(int k = j+1; k < this->g->num_vertices; ++k){
                    if(this->g->get_weight(i,k) > 0 && this->g->get_weight(j,k) <= 0 && this->g->get_weight(i,k) != DO_NOT_DELETE){
                        p3_list.emplace_back(i,j,k);
                    }
                }
            }
        }
    }
    return p3_list;
}
std::tuple<int, int, int> Solver::find_first_p3() {
    //std::cout <<"startwerte "<< i << " " << j << std::endl;
    for(int i = 0; i < this->g->num_vertices; ++i){
        for(int j = 0;j < this->g->num_vertices; ++j) {
            //      std::cout << i << " "<< j << std::endl;
            if ( this->g->get_weight(i,j) > 0) {
                for (int k = j + 1; k < this->g->num_vertices; ++k) {
                    if (this->g->get_weight(i,k) > 0 && (this->g->get_weight(j,k) <= 0)) {
                        return std::make_tuple(i, k, j);
                    }
                }
            }
        }
    }
    return std::make_tuple(-1,-1, -1);
}

std::tuple<int, int, int> Solver::get_max_cost_p3(int *max_cost){
    int first_tuple_val = -1;
    int second_tuple_val = -1;
    int third_tuple_val = -1;
    *max_cost = INT32_MIN;
    for(int i = 0; i < this->g->num_vertices; ++i){
        for(int j = 0; j < this->g->num_vertices; ++j){
            if(this->g->get_weight(i,j) > 0){
                for(int k = j+1; k < this->g->num_vertices; ++k){
                    if(this->g->get_weight(i,k) > 0 && this->g->get_weight(j,k) <= 0){
  /*                      int current_cost = 0;
                        int weight_i_k = abs(g->get_weight(i,k));
                        int weight_i_j = abs(g->get_weight(i,j));
                        int weight_j_k = abs(g->get_weight(j,k));
                        if(weight_i_j != DO_NOT_DELETE)
                            current_cost += weight_i_j;
                        if(weight_j_k != DO_NOT_DELETE)
                            current_cost += weight_j_k;
                        current_cost += weight_i_k;
*/
                        int current_cost = abs(g->get_weight(i,k)) + abs(g->get_weight(i,j)) + abs(g->get_weight(j,k));
                        current_cost /= 3;
                        if(current_cost > *max_cost) {
                            *max_cost = current_cost;
                            first_tuple_val = i;
                            second_tuple_val = j;
                            third_tuple_val = k;
                        }
                    }
                }
            }
        }
    }
#ifdef DEBUG
    //    std::cout << "counter " << counter<< " Found " << p3_list.size() << " p3's"<< std::endl;
#endif
    return std::make_tuple(first_tuple_val, second_tuple_val, third_tuple_val);
}

std::tuple<int, int, int> Solver::get_max_cost_p3_experimental(int *p3_weight){
    int counter = 0;
    int first_p3_vertex = -1;
    int sec_p3_vertex = -1;
    int thrd_p3_vertex = -1;
    *p3_weight = INT32_MIN;
//    std::list<std::tuple<int, int, int>> p3_list;
    for(int i = 0; i < g->num_vertices-1; ++i){
        counter++;
        for(int j = i+1; j < g->num_vertices; ++j){
            counter++;
            if(g->get_weight(i,j) > 0){
                for(int k = i+1; k < g->num_vertices; ++k){
                    counter++;
                    if(g->get_weight(k,j) > 0 && g->get_weight(i,k) <= 0){
                        int costs = abs(g->get_weight(i,k)) + abs(g->get_weight(i,j)) + abs(g->get_weight(k,j))/3;
                        if(costs > *p3_weight){
                            *p3_weight = costs;
                            first_p3_vertex = j;
                            sec_p3_vertex = i;
                            thrd_p3_vertex = k;
                        }
//                        p3_list.push_front(std::make_tuple(i,j,k));
                    }else if(k < j && g->get_weight(k,j) < 0 && g->get_weight(i,k) > 0 ){
//                        std::cout <<  val << " bei j = " << j+1 << " neighbourIdx "<< i << " " << k << std::endl;
                        int costs = abs(g->get_weight(i,k)) + abs(g->get_weight(i,j)) + abs(g->get_weight(k,j));
                        if(costs > *p3_weight){
                            *p3_weight = costs;
                            first_p3_vertex = i;
                            sec_p3_vertex = j;
                            thrd_p3_vertex = k;
                        }
//                        p3_list.push_front(std::make_tuple(i,k,j));
                    }
                }
            }
        }
    }
#ifdef DEBUG
    // std::cout << "counter " << counter<< " Found " << p3_list.size() << " p3's"<< std::endl;
#endif
    return std::make_tuple(first_p3_vertex,sec_p3_vertex,thrd_p3_vertex);
//    p3_list.push_front(max_cost_tuple);
//    return p3_list;
}


void Solver::printDebug(std::string line){
#ifdef DEBUG
    std::cout << line << std::endl;
#endif
}
