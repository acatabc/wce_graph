//
// Created by Julia Henkel on 20.04.21.
//

#include <tuple>
#include "Solver.h"
#include "iostream"
#define NONE -1
#define CLUSTER_GRAPH -2
#define DO_NOT_DELETE INT32_MAX
#define DO_NOT_ADD INT32_MIN
#define PRINTDEBUG true



void print_tuple4(std::tuple<int,int,int,int> a){
    std::cout << std::get<0>(a)<< " " << std::get<1>(a) << " " << std::get<2>(a)<< std::get<3>(a)<< std::endl;
}


void printList(std::list<std::tuple<int, int, int, int>> mylist){
#ifdef DEBUG
    if(PRINTDEBUG == true){
        std::list<std::tuple<int, int, int, int>>::iterator it;
        std::cout << "---------\n";
        std::cout << "mylist contains:\n";
        for(it=mylist.begin(); it!=mylist.end(); ++it){
            std::cout << "(" << std::get<0>(*it) << "," << std::get<1>(*it) <<"," << std::get<2>(*it) << ") cost " << std::get<3>(*it) << "\n";
//            std::cout << "(" << (std::get<0>(*it)+1) << "," << (std::get<1>(*it)+1) <<"," << (std::get<2>(*it)+1)  << ") cost " << std::get<3>(*it) << "\n";
        }
        std::cout << "list end\n";
        std::cout << "---------\n";
    }
#endif
}

bool comparator_P3_costs (std::tuple<int, int, int, int> first, std::tuple<int, int, int, int> second)
{
    return std::get<3>(first) >= std::get<3>(second);
}

Solver::Solver(WCE_Graph *graph): g(graph){}

Solver::~Solver() {}

void Solver::solve() {

    printDebug("\n\n------DEFAULT SOLVER--------------------");

    int k = 0;
    while (this->branch(k, 0) == NONE){
        k++;
        printDebug("\nSOLVE FOR k:" + std::to_string(k));
    }
    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(k) + "\n");

}

int Solver::branch(int k, int layer){
    rec_steps++;
    if(k < 0){
        return NONE;
    }

    auto p3 = this->get_max_cost_p3();

    if(std::get<0>(p3) == -1){
        return CLUSTER_GRAPH;
    }

    int v = std::get<0>(p3);
    int w = std::get<1>(p3);
    int u = std::get<2>(p3);

    if(this->branchEdge(u,v,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(v,w,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(w,u,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;

//    rec_steps--;
    return NONE;
}

int Solver::branchEdge(int u, int v, int k, int layer){
    printDebug("Layer " + std::to_string(layer) + " Edge " + std::to_string(u) + ", " + std::to_string(v));
    int weight = g->get_weight(u,v);

    if(weight == DO_NOT_DELETE || weight == DO_NOT_ADD) {
        printDebug("Do not delete / add " + std::to_string(u) + ", " + std::to_string(v));
        return NONE;
    }

    if(weight > 0) g->delete_edge(u, v);
    if(weight < 0) g->add_edge(u, v);

    if(this->branch(k-abs(weight), layer) == CLUSTER_GRAPH){
        std::cout << u+1 << " " << v+1 <<std::endl;
        return CLUSTER_GRAPH;
    }

    g->set_weight(u, v, weight);

    return NONE;
}

void Solver::solve2() {
    printDebug("\n\n------NEW SOLVER--------------------");

    add_all_p3_to_list();
    p3_list.sort(comparator_P3_costs);
    printList(p3_list);

    int k = 0;
    while (this->branch2(k, 0) == NONE){
        k++;
        printDebug("\nSOLVE FOR k:" + std::to_string(k));
//        printList(p3_list);
    }
    std::cout << "#recursive steps: " << rec_steps << std::endl;
    printDebug("final k:" + std::to_string(k) + "\n");
}

int Solver::branch2(int k, int layer){
    rec_steps++;
    if(k < 0){
        return NONE;
    }

    if(p3_list.empty()) return CLUSTER_GRAPH;

    // get first p3 in list, i.e. p3 with highest cost
    auto p3 = p3_list.front();

//    std::cout << "(" << (std::get<0>(p3)+1) << "," << (std::get<1>(p3)+1) <<"," << (std::get<2>(p3)+1)  << ") cost " << std::get<3>(p3) << "\n";
//    std::cout << "(" << (std::get<0>(p3)) << "," << (std::get<1>(p3)) <<"," << (std::get<2>(p3))  << ") cost " << std::get<3>(p3) << "\n";


    int v = std::get<0>(p3);
    int w = std::get<1>(p3);
    int u = std::get<2>(p3);

    if(this->branchEdge2(u,v,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge2(v,w,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge2(w,u,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;

//    rec_steps--;
    return NONE;
}

int Solver::branchEdge2(int u, int v, int k, int layer){
    printDebug("Layer " + std::to_string(layer) + " Edge " + std::to_string(u) + ", " + std::to_string(v));
    int weight = g->get_weight(u,v);

    if(weight == DO_NOT_DELETE || weight == DO_NOT_ADD) {
        printDebug("Do not delete / add " + std::to_string(u) + ", " + std::to_string(v));
        return NONE;
    }

    if(k-abs(weight) < 0) return NONE;  // put this here to avoid unnecessary updating of p3_list

    if(weight > 0) g->delete_edge(u, v);
    if(weight < 0) g->add_edge(u, v);

    update_p3_list(u,v);

    if(this->branch2(k-abs(weight), layer) == CLUSTER_GRAPH){
        std::cout << u+1 << " " << v+1 <<std::endl;
        return CLUSTER_GRAPH;
    }

    // reset weights and p3 list
    g->set_weight(u, v, weight);
    update_p3_list(u,v);

    return NONE;
}

// removes and adds p3's in list when edge (u,v) was modified
void Solver::update_p3_list(int u, int v){

    // remove all p3 that contain edge (u,v)
    for(iterator=p3_list.begin(); iterator != p3_list.end(); ++iterator){
        if((std::get<0>(*iterator) == u || std::get<1>(*iterator) == u || std::get<2>(*iterator) == u) && (std::get<0>(*iterator) == v || std::get<1>(*iterator) == v || std::get<2>(*iterator) == v)){
            p3_list.erase(iterator);
        }
    }

    int weight_uv = g->get_weight(u,v);

    // add all new p3, which are the result of modifying (u,v)
    // case 1: (u,v) was deleted
    if(weight_uv < 0){
        // we have a new p3, if u and v have the same neighbor
        for(int i = 0; i < g->num_vertices; i++){
            if(u == i || v == i) continue;
            int weight_ui = g->get_weight(u,i);
            int weight_vi = g->get_weight(v,i);
            if(weight_ui > 0 && weight_vi>0) {
                int cost = 0;
                if(weight_uv != DO_NOT_DELETE && weight_uv != DO_NOT_ADD) cost += abs(weight_uv);
                if(weight_ui != DO_NOT_DELETE && weight_ui != DO_NOT_ADD) cost += abs(weight_ui);
                if(weight_vi != DO_NOT_DELETE && weight_vi != DO_NOT_ADD) cost += abs(weight_vi);
                insert_p3_in_list(u,v,i,cost);
            }
        }
    }
    // case 2: (u,v) was added
    if(weight_uv > 0){
        // we have a new p3, if u has a neighbor which v does not have or reverse
        for(int i = 0; i < g->num_vertices; i++){
            if(u == i || v == i) continue;
            int weight_ui = g->get_weight(u,i);
            int weight_vi = g->get_weight(v,i);
            if((weight_ui > 0 && weight_vi<0)||(weight_ui < 0 && weight_vi>0)) {
                int cost = 0;
                if(weight_uv != DO_NOT_DELETE && weight_uv != DO_NOT_ADD) cost += abs(weight_uv);
                if(weight_ui != DO_NOT_DELETE && weight_ui != DO_NOT_ADD) cost += abs(weight_ui);
                if(weight_vi != DO_NOT_DELETE && weight_vi != DO_NOT_ADD) cost += abs(weight_vi);
                insert_p3_in_list(u,v,i,cost);
            }
        }
    }
}

void Solver::insert_p3_in_list(int u, int v, int w, int cost){
    for(iterator=p3_list.begin(); iterator != p3_list.end(); iterator++){
        if(cost > std::get<3>(*iterator)) {
            p3_list.insert(iterator, std::make_tuple(u, v, w, cost));
            break;
        }
    }
    // insert at the end of list
    if(iterator == p3_list.end()) p3_list.insert(iterator, std::make_tuple(u, v, w, cost));

    return;

}



int Solver::branchEdgeAllowLoop(int u, int v, int k, int layer){
//    printDebug("Layer " + std::to_string(layer) + " Edge " + std::to_string(u) + ", " + std::to_string(v));
    int weight = g->get_weight(u,v);

    g->modify_edge(u, v);
    if(this->branch(k-abs(weight), layer) == CLUSTER_GRAPH){
        std::cout << u+1 << " " << v+1 <<std::endl;
        return CLUSTER_GRAPH;
    }

    g->set_weight(u, v, weight);
    return NONE;
}

std::tuple<int, int, int> Solver::find_first_p3() {
    for(int i = 0; i < this->g->num_vertices; ++i){
        for(int j = 0;j < this->g->num_vertices; ++j) {
            if ( this->g->get_weight(i,j) > 0) {
                for (int k = j + 1; k < this->g->num_vertices; ++k) {
                    if (this->g->get_weight(i,k) > 0 && (this->g->get_weight(j,k) < 0)) {
                        return std::make_tuple(i, k, j);
                    }
                }
            }
        }
    }
    return std::make_tuple(-1,-1, -1);
}

void Solver::add_all_p3_to_list(){
    for(int i = 0; i < this->g->num_vertices; ++i){
        for(int j = 0; j < this->g->num_vertices; ++j){
            if(this->g->get_weight(i,j) > 0){
                for(int k = j+1; k < this->g->num_vertices; ++k){
                    if(this->g->get_weight(i,k) > 0 && this->g->get_weight(j,k) < 0){

                        // sum up costs of all three edges (only edges that are allowed to be modified)
                        int cost = 0;
                        if(this->g->get_weight(i,k) != DO_NOT_DELETE && this->g->get_weight(i,k) != DO_NOT_ADD) cost += abs(g->get_weight(i,k));
                        if(this->g->get_weight(i,j) != DO_NOT_DELETE && this->g->get_weight(i,j) != DO_NOT_ADD) cost += abs(g->get_weight(i,j));
                        if(this->g->get_weight(j,k) != DO_NOT_DELETE && this->g->get_weight(j,k) != DO_NOT_ADD) cost += abs(g->get_weight(j,k));

                        // add new p3 to list
                        p3_list.push_back(std::make_tuple(i, j, k, cost));
                    }
                }
            }
        }
    }
}



std::tuple<int, int, int> Solver::get_max_cost_p3(){
    int first_tuple_val = -1;
    int second_tuple_val = -1;
    int third_tuple_val = -1;
    int max_cost = INT32_MIN;
    for(int i = 0; i < this->g->num_vertices; ++i){
        for(int j = 0; j < this->g->num_vertices; ++j){
            if(this->g->get_weight(i,j) > 0){
                for(int k = j+1; k < this->g->num_vertices; ++k){
                    if(this->g->get_weight(i,k) > 0 && this->g->get_weight(j,k) < 0){

                        // sum up costs of all three edges (only edges that are allowed to be modified)
                        int current_cost = 0;
                        if(this->g->get_weight(i,k) != DO_NOT_DELETE && this->g->get_weight(i,k) != DO_NOT_ADD) current_cost += abs(g->get_weight(i,k));
                        if(this->g->get_weight(i,j) != DO_NOT_DELETE && this->g->get_weight(i,j) != DO_NOT_ADD) current_cost += abs(g->get_weight(i,j));
                        if(this->g->get_weight(j,k) != DO_NOT_DELETE && this->g->get_weight(j,k) != DO_NOT_ADD) current_cost += abs(g->get_weight(j,k));

                        // update maximum cost and corresponding p3
                        if(current_cost > max_cost) {
                            max_cost = current_cost;
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

void Solver::printDebug(std::string line){
#ifdef DEBUG
if(PRINTDEBUG == true){
    std::cout << line << std::endl;
}
#endif
}





/*
 *
 * save all p3 with corresponding costs in a list O(n^3)
 * sort list by costs 0(n^2)
 *
 * branch at some edge e of p3 with highest cost
 *      update p3s: update all p3s that in include edge e O(#p3s) = O(n über 3) = 0(n^3) -> but normally less than iterating all vertices neighbors and neighbors
 *
 *
 *
 */





