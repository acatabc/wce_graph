
#ifndef ALGENG_WCE_SOLVER_SOLVER_H
#define ALGENG_WCE_SOLVER_SOLVER_H

#include <vector>
#include <map>
#include "WCE_Graph.h"

class Solver {
private:
    unsigned int rec_steps = 0;
    WCE_Graph *g;
public:
    Solver();
    virtual ~Solver();
    static WCE_Graph *parse_and_build_graph();

    //solving
    void solve();
    int branch(int k, int layer);
    void final_output(int u, int v);
    void output_data_reduction();

    //method related to p3s
    std::tuple<int, int, int> get_max_cost_p3_naive();

    std::tuple<std::tuple<int, int, int>, int> get_best_p3_and_lowerBound_improved();
    struct p3{
        int i,j,k;
        int cost_sum;
        int min_cost;
    };
    struct p3 generate_p3_struct(int,int,int);
    std::vector<p3> find_all_p3_faster();


    // data reduction
    int data_reduction(int k, int layer);
    int data_reduction_before_branching();
    void undo_data_reduction(int final);

    int dataRed_weight_larger_k(int k);
    int dataRed_heavy_non_edge_branch(int k);
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
    int min_cut(const std::list<int>&);
    static int min_cut_phase(std::vector<std::vector<std::pair<int,int>>> &G, bool *active, std::vector<int> &active_nodes,
                             unsigned int num_possible_vertices);
    static std::pair<int,int> get_most_tightly_connected(const std::vector<int>& vertex_set, const bool *, std::vector<std::vector<std::pair<int,int>>>&, const bool *);

    // merging
    int unmerge_and_output(int uv);
    void clear_stack_and_output();

    // debug
    void verify_clusterGraph();
};
#endif //ALGENG_WCE_SOLVER_SOLVER_H
