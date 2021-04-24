
#include "Solver2.h"
#include <tuple>
#include "iostream"


Solver2::Solver2(WCE_Graph *graph): g(graph){}

Solver2::~Solver2() {}

void Solver2::solve() {
    printDebug("\n\n------NEW SOLVER--------------------");

    find_all_P3();
    p3List.sort(comparator_P3s);
    printList(p3List);

    int k = 0;
    while (this->branch(k, 0) == NONE){
        k++;
        printDebug("\nSOLVE FOR k:" + std::to_string(k));
//        printList(p3List);
    }
    std::cout << "#recursive steps: " << rec_steps << std::endl;
    printDebug("final k:" + std::to_string(k) + "\n");
}

int Solver2::branch(int k, int layer){
    rec_steps++;
    if(k < 0){
        return NONE;
    }

    if(p3List.empty()) return CLUSTER_GRAPH;

    // get first p3 in list, i.e. p3 with highest cost
    auto p3 = p3List.front();

//    std::cout << "(" << (std::get<0>(p3)+1) << "," << (std::get<1>(p3)+1) <<"," << (std::get<2>(p3)+1)  << ") cost " << std::get<3>(p3) << "\n";
//    std::cout << "(" << (std::get<0>(p3)) << "," << (std::get<1>(p3)) <<"," << (std::get<2>(p3))  << ") cost " << std::get<3>(p3) << "\n";


    int v = std::get<0>(p3);
    int w = std::get<1>(p3);
    int u = std::get<2>(p3);

    if(this->branchEdge(u,v,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(v,w,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(w,u,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;

//    rec_steps--;
    return NONE;
}

int Solver2::branchEdge(int u, int v, int k, int layer){
    printDebug("Layer " + std::to_string(layer) + " Edge " + std::to_string(u) + ", " + std::to_string(v));
    int weight = g->get_weight(u,v);

    if(weight == DO_NOT_DELETE || weight == DO_NOT_ADD) {
        printDebug("Do not delete / add " + std::to_string(u) + ", " + std::to_string(v));
        return NONE;
    }

    if(k-abs(weight) < 0) return NONE;  // put this here to avoid unnecessary updating of p3List

    if(weight > 0) g->delete_edge(u, v);
    if(weight < 0) g->add_edge(u, v);

    update_p3List(u, v);

    if(this->branch(k-abs(weight), layer) == CLUSTER_GRAPH){
        std::cout << u+1 << " " << v+1 <<std::endl;
        return CLUSTER_GRAPH;
    }

    // reset weights and p3 list
    g->set_weight(u, v, weight);
    update_p3List(u, v);

    return NONE;
}

// removes and adds p3's in p3List when edge (u,v) was modified
void Solver2::update_p3List(int u, int v){

    // remove all p3 that contain edge (u,v)
    for(iterator=p3List.begin(); iterator != p3List.end(); ++iterator){
        if((std::get<0>(*iterator) == u || std::get<1>(*iterator) == u || std::get<2>(*iterator) == u) && (std::get<0>(*iterator) == v || std::get<1>(*iterator) == v || std::get<2>(*iterator) == v)){
            iterator = p3List.erase(iterator);
            iterator--;
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
                insert_p3(u, v, i, cost);
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
                insert_p3(u, v, i, cost);
            }
        }
    }
}

// inserts given p3 in p3List at correct position
void Solver2::insert_p3(int u, int v, int w, int cost){
    for(iterator=p3List.begin(); iterator != p3List.end(); iterator++){
        if(cost > std::get<3>(*iterator)) {
            p3List.insert(iterator, std::make_tuple(u, v, w, cost));
            break;
        }
    }
    // insert at the end of list
    if(iterator == p3List.end()) p3List.insert(iterator, std::make_tuple(u, v, w, cost));

    return;

}

// finds all p3s and adds them to p3List
void Solver2::find_all_P3(){
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
                        p3List.push_back(std::make_tuple(i, j, k, cost));
                    }
                }
            }
        }
    }
}

// returns P3 with higher cost
bool Solver2::comparator_P3s (std::tuple<int, int, int, int> first, std::tuple<int, int, int, int> second)
{
    return std::get<3>(first) >= std::get<3>(second);
}