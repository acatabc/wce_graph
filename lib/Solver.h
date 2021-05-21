
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
    int function();
    void final_output(int u, int v);
    void output_data_reduction();

    //method related to p3s
    std::tuple<int, int, int> get_max_cost_p3_naive();

    // data reduction
    int data_reduction(int k, int layer);
    void undo_data_reduction(int final);

    int dataRed_weight_larger_k(int k);
    void dataRed_heavy_non_edge();
    int dataRed_heavy_non_edge_branch(int k);
    int dataRed_heavy_edge_single_end(int k);
    int dataRed_heavy_edge_single_end_branch(int k);
    int dataRed_heavy_edge_both_ends(int k);
    int dataRed_remove_existing_clique();
    void DFS(int , bool *, std::vector<int>&);
    int dataRed_merge_dnd(int k);

    int dataRed_large_neighbourhood_I(int);
    //return pair for neighbourhood(first), & not neighbours
    std::pair<std::list<int>, std::list<int>> closed_neighbourhood(int u);
    int deficiency(std::list<int> neighbours);
    int cut_weight(std::list<int>& neighbourhood, std::list<int>& rest_graph);

    // merging
    int unmerge_and_output(int uv);
    void clear_stack_and_output();

    // debug
    void verify_clusterGraph();


};


#endif //ALGENG_WCE_SOLVER_SOLVER_H
