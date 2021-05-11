
#ifndef ALGENG_WCE_SOLVER_SOLVER_H
#define ALGENG_WCE_SOLVER_SOLVER_H

#include <vector>
#include "WCE_Graph.h"

class Solver {
private:
    unsigned int rec_steps = 0;
    WCE_Graph *g;
public:

    Solver();
    virtual ~Solver();

    WCE_Graph *parse_and_build_graph();

    //solving
    void solve();
    int branch(int k, int layer);
    int branchEdge(int u, int v, int k, int layer);
    void final_output(int u, int v);

    //method related to p3s
    std::tuple<int, int, int> get_max_cost_p3_naive();


    // data reduction methods
    int data_reduction(int k);
    int dataRed_weight_larger_k(int k);
    void dataRed_heavy_non_edge();
    int dataRed_heavy_edge_single_end();

    //large neighbourhood
    int dataRed_large_neighbourhood_I();
    //return pair for neighbourhood(first), & not neighbours
    std::pair<std::list<int>, std::list<int>> closed_neighbourhood(int u);
    int deficiency(std::list<int> neighbours);
    int cut_weight(std::list<int>& neighbourhood, std::list<int>& rest_graph);

    // merging
    int unmerge_and_output(int uv);
    void final_unmerge_and_output();


    // debug
    void verify_clusterGraph();
    std::vector<int> k_forward;
    std::vector<int> k_backward;

};


#endif //ALGENG_WCE_SOLVER_SOLVER_H
