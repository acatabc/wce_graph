
#ifndef ALGENG_WCE_SOLVER_SOLVER2_H
#define ALGENG_WCE_SOLVER_SOLVER2_H

#include "WCE_Graph.h"
#include "../include/utils.h"

class Solver2 {
private:
    WCE_Graph *g;
    unsigned int rec_steps = 0;

    std::list<std::tuple<int, int, int, int>> p3List;
    std::list<std::tuple<int, int, int, int>>::iterator iterator;
    void find_all_P3();

public:
    Solver2(WCE_Graph *);
    virtual ~Solver2();

    void solve();
    int branch(int k, int layer);
    int branchEdge(int u, int v, int k, int layer);

    void update_p3List(int u, int v);
    void insert_p3(int u, int v, int w, int cost);

    static bool comparator_P3s(std::tuple<int, int, int, int> first, std::tuple<int, int, int, int> second);

};


#endif //ALGENG_WCE_SOLVER_SOLVER2_H
