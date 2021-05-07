//
// Created by Julia Henkel on 20.04.21.
//

#include <tuple>
#include "Solver2.h"
#include "iostream"
#define NONE -1
#define CLUSTER_GRAPH -2
#define DO_NOT_DELETE INT32_MAX
#define DO_NOT_ADD INT32_MIN

#define FORWARD 0
#define BACKWARD 1

#define PRINTDEBUG true
WCE_Graph *Solver2::parse_and_build_graph(){
#ifdef DEBUG
    //file test_data.txt > stdin
    //I dont like the file path thing but ok...
    //std::cout <<"../test_data/test_data.txt" << std::endl;
    freopen("../test_data/a001.dimacs", "r", stdin);
#endif
    int num_vertices = 0;
    std::cin >> num_vertices;
    WCE_Graph *g = new WCE_Graph(num_vertices);
    int v, w, weight;
    while(std::cin){
        std::cin >> v >> w >> weight;
        v -= 1;
        w -= 1;
        if(!std::cin.fail())
            g->set_weight(v,w, weight);
    }
    return g;
}
Solver2::Solver2(){
    this->g = this->parse_and_build_graph();
    p3s.resize(g->num_vertices);
}

Solver2::~Solver2() {}

void Solver2::solve() {
    int k = 0;
    this->get_all_p3(); //n^3
    while (this->branch(k, 0) == NONE){
        k++;
//        printDebug("\nSOLVE FOR k:" + std::to_string(k));
    }
    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(k) + "\n");
}

int Solver2::branch(int k, int layer){
    if(k < 0){
        return NONE;
    }

//    auto p3 = this->get_rand_p3(); //O(n) worst case
    auto p3 = this->get_max_p3(); //O(n)

    if(std::get<0>(p3) == -1){
        return CLUSTER_GRAPH;
    }
    rec_steps++;

    int v = std::get<0>(p3);
    int w = std::get<1>(p3);
    int u = std::get<2>(p3);
//    printDebug(std::to_string(layer) + ": " + std::to_string(v) + " "+ std::to_string(w) + " "+ std::to_string(u));
    if(this->branchEdge(u,v,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(v,w,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(w,u,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;

    return NONE;
}

int Solver2::branchEdge(int u, int v, int k, int layer){
//    printDebug("Layer " + std::to_string(layer) + " Edge " + std::to_string(u) + ", " + std::to_string(v));
    int weight = g->get_weight(u,v);

    if(weight == DO_NOT_DELETE || weight == DO_NOT_ADD) {
//        printDebug("Do not delete / add " + std::to_string(u) + ", " + std::to_string(v));
        return NONE;
    }

    if(weight > 0) g->delete_edge(u, v);
    if(weight < 0) g->add_edge(u, v);
    update_p3s(u,v, weight, FORWARD); //n*log(n^2)

    if(this->branch(k-abs(weight), layer) == CLUSTER_GRAPH){
        std::cout << u+1 << " " << v+1 <<std::endl;
        return CLUSTER_GRAPH;
    }

    g->set_weight(u, v, weight);
    update_p3s(u,v, weight, BACKWARD); //n*log(n^2)

    return NONE;
}

std::tuple<int, int, int> Solver2::get_max_cost_p3(){
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


void Solver2::get_all_p3() {
    for(int i = 0; i < this->g->num_vertices; ++i){
        for(int j = 0; j < this->g->num_vertices; ++j){
            if(this->g->get_weight(i,j) > 0){
                for(int k = j+1; k < this->g->num_vertices; ++k){
                    if(this->g->get_weight(i,k) > 0 && this->g->get_weight(j,k) < 0){
                        this->add_p3(i,j,k);
                    }
                }
            }
        }
    }
}
//v --- u --- w
void Solver2::add_p3(int u, int v, int w) {
    p3s.at(u).emplace(p3(u,v,w,g->get_costs(u,v,w)), std::make_pair(v,w));
}

void Solver2::remove_p3(int u, int v, int w, int old_weight, int flag){
    //some magic happening here, to calculate the correct costs of a p3, because after changing an edge a different value for the cost is
    //calculated. To delete an p3 from the p3s trees we need the cost when it was inserted as a p3, so we need the costs of the edge
    //before the edge was deleted or inserted
    int weight = 0;
    int weight_uv = g->get_weight(u,v);
    int weight_uw = g->get_weight(u,w);
    int weight_vw = g->get_weight(v,w);
    if(weight_uv != DO_NOT_DELETE && weight_uv != DO_NOT_ADD){
        weight += abs(weight_uv);
    }
    if(weight_uw != DO_NOT_DELETE && weight_uw != DO_NOT_ADD){
        weight += abs(weight_uw);
    }
    if(weight_vw != DO_NOT_DELETE && weight_vw != DO_NOT_ADD){
        weight += abs(weight_vw);
    }
    if(flag == BACKWARD)
        weight -= abs(old_weight);
    else
        weight += abs(old_weight);
//    p3s.at(u).erase(std::make_tuple(u,v,w,weight));
    p3s.at(u).erase(p3(u,v,w,weight));
}

std::tuple<int,int,int> Solver2::get_rand_p3() {
    int u = rand() % p3s.size();
    auto p3_of_u = p3s.at(u);
    int counter = 0;
    while(p3_of_u.empty() && counter <= p3s.size()){
        u = (u+1) % p3s.size();
        p3_of_u = p3s.at(u);
        printDebug("hello"+ std::to_string(counter));
        counter++;
    }
    if(counter >= p3s.size())
        return std::tuple<int, int, int>(-1, -1, -1);
    auto pair = *p3_of_u.begin();
    printDebug(std::to_string(pair.second.first) + " "+ std::to_string(pair.second.second));
    return std::tuple<int,int,int>(u, pair.second.first, pair.second.second);
}

std::tuple<int, int, int> Solver2::get_max_p3() {
    auto max_tuple = std::make_tuple(-1,-1,-1);
    int max_weight = -1;
    int u = 0;
    for(auto& a : p3s){
        for(auto& tup: a) {
            int v = tup.second.first;
            int w = tup.second.second;
            int _max_weight = g->get_costs(u, v, w);
//            printDebug(std::to_string(v) + " " + std::to_string(w) + " " + std::to_string(u) + " "+ std::to_string(_max_weight));
            if (_max_weight > max_weight) {
                max_tuple = std::make_tuple(u, v, w);
                max_weight = _max_weight;
            }
            break;  //so we just get the first value in the tree, the root. the root has the highest cost
        }
        u++;
    }
    return max_tuple;
}

void Solver2::update_p3s(int u, int v, int old_weight, int flag) {
    //edge was removed
    if(g->get_weight(u,v) < 0){
        for(int i = 0; i < g->num_vertices; ++i){
            if(g->get_weight(u,i) > 0 && g->get_weight(v,i) < 0)
                //remove
                remove_p3(u,i,v, old_weight, flag);
            else if(g->get_weight(u,i) > 0 && g->get_weight(v,i) > 0)
                add_p3(i,v,u);

            if(g->get_weight(v,i) > 0 && g->get_weight(u,i) < 0)
                remove_p3(v,i,u, old_weight, flag);
        }
    } else if(g->get_weight(u,v) > 0){
        //edge was added
        for(int i = 0; i < g->num_vertices; ++i){
            if(g->get_weight(u,i) > 0 && g->get_weight(v,i) < 0)
                add_p3(u,i,v);
            else if(g->get_weight(u,i) > 0 && g->get_weight(v,i) > 0)
                remove_p3(i,v,u, old_weight, flag);

            if(g->get_weight(v,i) > 0 && g->get_weight(u,i) < 0)
                add_p3(v,i,u);
        }
    }
}

/*+++++++++++DEBUG SECTION +++++++++++++++++++++*/
void Solver2::printDebug(std::string line){
#ifdef DEBUG
if(PRINTDEBUG == true){
    std::cout << line << std::endl;
}
#endif
}
bool operator<(const Solver2::p3& a, const Solver2::p3& b){
    if(a.cost == b.cost){
        return (a.cost+a.v+a.w) > (b.cost+b.v+b.w);
    }else{
        return a.cost > b.cost;
    }
}
