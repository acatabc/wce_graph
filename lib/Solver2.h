//
// Created by Julia Henkel on 20.04.21.
//

#ifndef ALGENG_WCE_SOLVER_SOLVER2_H
#define ALGENG_WCE_SOLVER_SOLVER2_H

#include <map>
#include <vector>
#include "WCE_Graph.h"

class Solver2 {
private:
    unsigned int rec_steps = 0;
public:
    struct p3{
        int u;
        int v;
        int w;
        int cost;
        p3(int u, int v, int w, int cost):u(u),v(v),w(w),cost(cost){}
    };
    struct p3_comp {
//        bool operator()(const std::tuple<int,int,int,int>& a , const std::tuple<int,int,int,int>& b){
//            int a_second = std::get<1>(a);
//            int a_third = std::get<2>(a);
//            int b_second = std::get<1>(b);
//            int b_third = std::get<2>(b);
//
//            int a_weight = std::get<3>(a);
//            int b_weight = std::get<3>(b);
//            if(a_weight == b_weight) {
//                a_weight += a_second +a_third;
//                b_weight += b_second +b_third;
//            }
//            return a_weight > b_weight;
//        }
        bool operator()(const p3& a, const p3& b){
            if(a.cost == b.cost){
                return (a.cost+a.v+a.w) > (b.cost+b.v+b.w);
            }else{
                return a.cost > b.cost;
            }
        }
    };

    WCE_Graph *g;
//    std::vector<std::map<std::tuple<int,int,int,int>, std::pair<int,int>, p3_comp>> p3s;
    std::vector<std::map<p3, std::pair<int,int>, p3_comp>> p3s;

    Solver2();
    virtual ~Solver2();
    WCE_Graph *parse_and_build_graph();

    //solving
    void solve();
    int branch(int k, int layer);
    int branchEdge(int u, int v, int k, int layer);

    //method related to p3s
    void get_all_p3();
    std::tuple<int,int,int> get_rand_p3();
    std::tuple<int,int,int> get_max_p3();
    void update_p3s(int, int, int, int);
    std::tuple<int, int, int> get_max_cost_p3();
    void add_p3(int,int,int);
    void remove_p3(int u, int v, int w, int weight, int flag);


    //Debug utils
    void printDebug(std::string);
};


#endif //ALGENG_WCE_SOLVER_SOLVER2_H
