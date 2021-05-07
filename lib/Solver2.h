
#ifndef ALGENG_WCE_SOLVER_SOLVER2_H
#define ALGENG_WCE_SOLVER_SOLVER2_H

#include <map>
#include <vector>
#include "WCE_Graph.h"

class Solver2 {
private:
    unsigned int rec_steps = 0;
    WCE_Graph *g;
public:

    Solver2();
    virtual ~Solver2();

    WCE_Graph *parse_and_build_graph();

    //p3 struct is used as key in the tree where we save the p3s
    struct p3{
        int u;
        int v;
        int w;
        int cost;
        p3(int u, int v, int w, int cost):u(u),v(v),w(w),cost(cost){}
    };

    //vector of trees of p3s (yes it says map of p3s but a map is using RB-trees internally)
    //a p3 v---u---w is stored in vector[u] with value (pair<int,int> = v,w) with the key: struct p3 u,v,w,cost
    std::vector<std::map<struct p3, std::pair<int,int>>> p3s;

    //solving
    void solve();
    int branch(int k, int layer);
    int branchEdge(int u, int v, int k, int layer);
    void final_output(int u, int v);

    //method related to p3s
    void find_all_p3s();
    void add_p3(int,int,int);
    void remove_p3(int u, int v, int w, int weight, int flag);
    std::tuple<int,int,int> get_rand_p3();
    std::tuple<int,int,int> get_max_cost_p3();
    void update_p3s(int, int, int, int);
    std::tuple<int, int, int> get_max_cost_p3_naive();


    // data reduction methods
    int dataRed_weight_larger_k(int k);

    // merging
    int unmerge_and_output(int uv);
    void final_unmerge_and_output();


    // debug
    void verify_clusterGraph();
    std::vector<int> k_forward;
    std::vector<int> k_backward;

};


#endif //ALGENG_WCE_SOLVER_SOLVER2_H
