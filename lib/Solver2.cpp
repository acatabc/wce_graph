
#include "Solver2.h"
#include <tuple>
#include "iostream"
#include "../include/utils.h"
#include <math.h>

#define FORWARD 0
#define BACKWARD 1

#define NONE -1
#define CLUSTER_GRAPH -2

Solver2::Solver2(){
    this->g = this->parse_and_build_graph();
}

Solver2::~Solver2() {}

void Solver2::solve() {
    int k = 0;
    int cluster_graph = NONE;
    while (cluster_graph == NONE){
        printDebug("\nSOLVE FOR k:" + std::to_string(k));

        int k_reduced = dataRed_weight_larger_k(k); // data reduction method

        if(k_reduced >= 0){
            this->find_all_p3s();
            cluster_graph = this->branch(k_reduced, 0);
        }

        if(cluster_graph == CLUSTER_GRAPH)
            final_unmerge_and_output();
        else{
            g->recover_original();
            k++;
            k_forward.clear();
        }

    }

    verify_clusterGraph();

    std::cout << "#recursive steps: " << rec_steps << std::endl;

    printDebug("final k:" + std::to_string(k) + "\n");
}


int Solver2::branch(int k, int layer){
    if(k < 0){
        return NONE;
    }

//    auto p3 = this->get_rand_p3(); //O(n) worst case
    auto p3 = this->get_max_cost_p3(); //O(n)

    if(std::get<0>(p3) == -1){
        printDebug("FOUND CLUSTER GRAPH");
        return CLUSTER_GRAPH;
    }
    rec_steps++;

    int v = std::get<0>(p3);
    int w = std::get<1>(p3);
    int u = std::get<2>(p3);

    if(this->branchEdge(u,v,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(v,w,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;
    if(this->branchEdge(w,u,k, layer+1) == CLUSTER_GRAPH) return CLUSTER_GRAPH;

    return NONE;
}

int Solver2::branchEdge(int u, int v, int k, int layer){
    printDebug("Layer " + std::to_string(layer) + " Edge " + std::to_string(u) + ", " + std::to_string(v));
    int weight = g->get_weight(u,v);

    if(weight == DO_NOT_DELETE || weight == DO_NOT_ADD) {
        printDebug("Do not delete / add " + std::to_string(u) + ", " + std::to_string(v));
        return NONE;
    }

    if(weight > 0) g->delete_edge(u, v);
    if(weight < 0) g->add_edge(u, v);

    update_p3s(u,v, weight, FORWARD); //n*log(n^2)

    if(this->branch(k-abs(weight), layer) == CLUSTER_GRAPH){
        if(u < g->num_vertices && v < g->num_vertices)
            final_output(u,v);
        else
            printDebug("output (later): " + std::to_string(u+1-1) + " " + std::to_string(v+1-1));
        return CLUSTER_GRAPH;
    }

    g->set_weight(u, v, weight);

    update_p3s(u,v, weight, BACKWARD); //n*log(n^2)

    return NONE;
}

void Solver2::final_output(int u, int v)
{
#ifdef DEBUG
    if(PRINTDEBUG){
//        std::cout << u+1-1 << " " << v+1-1 <<std::endl;
    }
#endif
    std::cout << u+1 << " " << v+1 <<std::endl;
}


// ----------------------------
// ------- p3 - search --------

// iterates all vertex tuples and inserts all p3 in p3vec
void Solver2::find_all_p3s() {
    for(int i = 0; i<p3s.size(); i++){
        p3s[i].clear();
    }
    p3s.resize(g->merge_map.size());
    for(int i: this->g->active_nodes){
        for(int j: this->g->active_nodes){
            for(int k : this->g->active_nodes){
                if(i == j || i == k || k == j) continue;
                if(this->g->get_weight(i,j) >= 0 && this->g->get_weight(i,k) >= 0 && this->g->get_weight(j,k) <= 0){
                    this->add_p3(i,j,k);
                }
            }
        }
    }
}

// adds (v,u,w) to p3vec
void Solver2::add_p3(int u, int v, int w) {
    p3s.at(u).emplace(p3(u,v,w, g->get_cost(u, v, w)), std::make_pair(v, w));
}

// removes (v,u,w) from p3-map
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


// updates vector of p3s given that u,v was modified (delete/add p3s)
// TODO added <= and >= (correct?)
void Solver2::update_p3s(int u, int v, int old_weight, int flag) {
    //edge was removed
    if(g->get_weight(u,v) <= 0){
        for(int i: this->g->active_nodes){
            if(i == v || i == u) continue;
            if(g->get_weight(u,i) > 0 && g->get_weight(v,i) < 0)
                remove_p3(u,i,v, old_weight, flag);
            if(g->get_weight(v,i) > 0 && g->get_weight(u,i) < 0)
                remove_p3(v,i,u, old_weight, flag);
            if(g->get_weight(u,i) >= 0 && g->get_weight(v,i) >= 0)
                add_p3(i,v,u);
        }
    } else if(g->get_weight(u,v) >= 0){
        //edge was added
        for(int i: this->g->active_nodes){
            if(i == v || i == u) continue;
            if(g->get_weight(u,i) >= 0 && g->get_weight(v,i) <= 0)
                add_p3(u,i,v);
            if(g->get_weight(v,i) >= 0 && g->get_weight(u,i) <= 0)
                add_p3(v,i,u);
            if(g->get_weight(u,i) > 0 && g->get_weight(v,i) > 0)
                remove_p3(i,v,u,old_weight, flag);
        }
    }
}

// returns a random p3 out of p3vec
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
//    printDebug(std::to_string(pair.second.first) + " "+ std::to_string(pair.second.second));
    return std::tuple<int,int,int>(u, pair.second.first, pair.second.second);
}

// returns p3 with max cost out of p3vec
std::tuple<int, int, int> Solver2::get_max_cost_p3() {
    auto max_tuple = std::make_tuple(-1,-1,-1);
    int max_weight = -1;
    int u = 0;
    for(auto& a : p3s){
        for(auto& tup: a) {
            int v = tup.second.first;
            int w = tup.second.second;
            int _max_weight = g->get_cost(u, v, w);
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

// iterates over all vertex tuples and returns max_cost p3
std::tuple<int, int, int> Solver2::get_max_cost_p3_naive(){
    int first_tuple_val = -1;
    int second_tuple_val = -1;
    int third_tuple_val = -1;
    int max_cost = INT32_MIN;
    for(int i: this->g->active_nodes){
        for(int j: this->g->active_nodes){
            for(int k : this->g->active_nodes){
                if(i == j || i == k || k == j) continue;
                if(this->g->get_weight(i,j) >= 0 && this->g->get_weight(i,k) >= 0 && this->g->get_weight(j,k) <= 0){

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
#ifdef DEBUG
    //    std::cout << "counter " << counter<< " Found " << p3_list.size() << " p3's"<< std::endl;
#endif
    return std::make_tuple(first_tuple_val, second_tuple_val, third_tuple_val);
}

// comparator returns p3 with higher cost
bool operator<(const Solver2::p3& a, const Solver2::p3& b){
    if(a.cost == b.cost){
        return (a.cost+a.v+a.w) > (b.cost+b.v+b.w);
    }else{
        return a.cost > b.cost;
    }
}



// ----------------------------
// ------- data reduction  --------

// continuously merges all vertices whose edge weight exceeds the available costs
// returns remaining costs k after merging
// if k<0 no solution for the graph and input k exists
int Solver2::dataRed_weight_larger_k(int k){
    printDebug("Data reduction (weight > k): ");
    int k_before = k;

    start:
    g->print_active_graph(std::cout);
    for(int u : g->active_nodes){
        for(int v : g->active_nodes){
            if(u == v) continue;
            if(g->get_weight(u,v) > k){ // for > -k set to -infinity TODO
                int kd = g->merge(u,v);
                k -= kd;
                k_forward.push_back(kd);
                if(k < 0) {
                    printDebug("Fail: maximum cost exceeded");
                    return -1;
                }
                goto start; // do this to avoid problems with modification of g->active_nodes TODO
            }
        }
    }

    if(k != k_before)
        printDebug("Reduced k to " + std::to_string(k));
    else
        printDebug("no edges to merge");

    return k;
}

// ----------------------------
// ------- merging --------

// unmerges all remaining vertices (after the solver finished) and outputs all modified edges
void Solver2::final_unmerge_and_output(){
    int k = 0;
    while (g->merge_map.size() != g->num_vertices){
        verify_clusterGraph();
        int uv = g->merge_map.size() - 1;
        int dk = unmerge_and_output(uv);
        k_backward.insert(k_backward.begin(), dk);
        k += dk;
    }
    printDebug("\nUnmerging sum of costs " +  std::to_string(k));

}

// unmerges vertex uv and outputs all modified edges (with u and v) to keep same edge connection as with uv
int Solver2::unmerge_and_output(int uv){
    std::vector<int> uv_children = g->merge_map[uv];
    int u = uv_children[0];
    int v = uv_children[1];

    printDebug("Output for unmerging (" + std::to_string(u) + "," + std::to_string(v) + ") -> " + std::to_string(uv));

    int dk = 0;
    for(int x : g->active_nodes) {
        if(x == uv || x == u ||x == v) continue;

        // NOTE: if w(u,x) == 0 we have to do something ----> delete or add depending on w(uv,x)
        if (g->get_weight(u, x) == 0 || (signbit(g->get_weight(u, x)) != signbit(g->get_weight(uv, x)))) {
            dk += abs(g->get_weight(u,x));

            // if w(uv,x) == 0 it doesnt matter if we delete or add ---> add edge
            if(g->get_weight(uv, x) >= 0)
                g->add_edge(u,x);
            if(g->get_weight(uv, x) < 0)
                g->delete_edge(u,x);

            if(u < g->num_vertices && x < g->num_vertices)
                final_output(u,x);
            else
                printDebug("Output queue: " + std::to_string(u+1-1) + " " + std::to_string(x+1-1));
        }
        if (g->get_weight(v, x) == 0 ||(signbit(g->get_weight(v, x)) != signbit(g->get_weight(uv, x)))) {
            dk += abs(g->get_weight(v,x));
            if(g->get_weight(uv, x) >= 0)
                g->add_edge(v,x);
            if(g->get_weight(uv, x) < 0)
                g->delete_edge(v,x);

            if(v < g->num_vertices && x < g->num_vertices)
                final_output(v,x);
            else
                printDebug("Output queue: " + std::to_string(v+1-1) + " " + std::to_string(x+1-1));

        }
    }
    printDebug("Unmerged " +  std::to_string(uv) + " -> (" + std::to_string(u) + "," + std::to_string(v) + ")" + " with cost " + std::to_string(dk));

    g->unmerge(uv);

    return dk;
}


WCE_Graph *Solver2::parse_and_build_graph(){
#ifdef DEBUG
    freopen("../wce-students/2-real-world/w033.dimacs", "r", stdin);
//    freopen("../test_data/a001.dimacs", "r", stdin);
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
    for(int i = 0; i< g->num_vertices; i++){
        std::vector<int> u = {i};
        g->merge_map.push_back(u);
        g->active_nodes.push_back(i);
    }
    return g;
}




// ------ Debug ----


// verify that the current graph is now a cluster graph
void Solver2::verify_clusterGraph(){
    auto p3 = this->get_max_cost_p3();
    if(std::get<0>(p3) == -1){
        printDebug("\nVERIFICATION SUCCESS\n");
    } else {
        printDebug("\nVERIFICATION FAIL:");
        print_tuple(p3);
    }
}