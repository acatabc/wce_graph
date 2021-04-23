//
// Created by Julia Henkel on 20.04.21.
//

#ifndef ALGENG_WCE_SOLVER_SOLVER_H
#define ALGENG_WCE_SOLVER_SOLVER_H

#include "WCE_Graph.h"

class Solver {
private:
    WCE_Graph *g;
    unsigned int rec_steps = 0;

    std::list<std::tuple<int, int, int, int>> p3_list;
    std::list<std::tuple<int, int, int, int>>::iterator iterator;
    void add_all_p3_to_list();

public:
    Solver(WCE_Graph *);
    virtual ~Solver();


    // new functions for solve2()
    void solve2();
    int branch2(int k, int layer);
    int branchEdge2(int u, int v, int k, int layer);
    void update_p3_list(int u, int v);
    void insert_p3_in_list(int u, int v, int w, int cost);
//    bool comparator_P3_costs(std::tuple<int, int, int, int> first, std::tuple<int, int, int, int> second);

    void solve();
    int branch(int k, int layer);
    int branchEdge(int u, int v, int k, int layer);

    int branchEdgeAllowLoop(int u, int v, int k, int layer);
    std::tuple<int, int, int> find_next_p3();
    std::tuple<int, int, int> find_first_p3();

    void print_all_p3();
    void printDebug(std::string);

    std::list<std::tuple<int, int, int>> get_all_p3();
    std::tuple<int, int, int> get_max_cost_p3();

    std::tuple<int, int, int> get_max_cost_p3_experimental(int *p3_weight);
};


#endif //ALGENG_WCE_SOLVER_SOLVER_H
