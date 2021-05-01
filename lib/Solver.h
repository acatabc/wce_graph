//
// Created by Julia Henkel on 20.04.21.
//

#ifndef ALGENG_WCE_SOLVER_SOLVER_H
#define ALGENG_WCE_SOLVER_SOLVER_H

#include <map>
#include <vector>
#include "WCE_Graph.h"

class Solver {
private:
    unsigned int rec_steps = 0;
public:
    WCE_Graph *g;
    std::vector<std::map<int, std::pair<int,int>, std::greater<>>> p3s;
    Solver();
    virtual ~Solver();
    WCE_Graph *parse_and_build_graph();

    //solving
    void solve();
    int branch(int k, int layer);
    int branchEdge(int u, int v, int k, int layer);

    //method related to p3s
    void get_all_p3();
    std::tuple<int,int,int> get_rand_p3();
    void update_p3s(int, int);
    std::tuple<int, int, int> get_max_cost_p3();
    void add_p3(int,int,int);
    void remove_p3(int u, int v, int w);

    //Debug utils
    void printDebug(std::string);
};


#endif //ALGENG_WCE_SOLVER_SOLVER_H
