
#ifndef ALGENG_WCE_SOLVER_SOLVER_H
#define ALGENG_WCE_SOLVER_SOLVER_H

#include <vector>
#include <map>
#include "WCE_Graph.h"


#define MAX_NUM_VERTICES 1000


class Solver {
private:
    unsigned int rec_steps = 0;
public:
    Solver();
    virtual ~Solver();
    WCE_Graph *parse_and_build_graph();
    WCE_Graph *g;

    //solving
    void deepS();
    int deepB(int c, int layer);
    int deep_data_reduction(int k, int layer);
    int get_lower_bound();

    int upperBound = 2000;
    int get_upper_bound();

    std::stack<WCE_Graph::stack_elem> best_solution_stack = std::stack<WCE_Graph::stack_elem>();
    void save_into_best_solution_stack(std::stack<WCE_Graph::stack_elem> current_stack);
    void output_from_best_solution_stack();



    void solve();
    int branch(int k, int layer);
    int branch_old(int k, int layer);
    void final_output(int u, int v);
    void output_data_reduction();

    //method related to p3s
    std::tuple<int, int, int> get_max_cost_p3_naive();
    std::tuple<std::tuple<int, int, int>, int> get_max_cost_p3_naive_lowerBound();
    std::tuple<std::tuple<int, int, int>, int> get_best_p3_and_lowerBound_improved();
    struct p3{
        int i,j,k;
        int cost_sum;
        int min_cost;
    };
    struct p3 generate_p3_struct(int,int,int);
    std::vector<p3> find_all_p3_faster();
    std::vector<p3> find_all_p3();


    // data reduction
    int data_reduction(int k, int layer);
    int data_reduction_before_branching();
    void undo_data_reduction(int final);

    int dataRed_weight_larger_k(int k);
    void dataRed_heavy_non_edge();
    int dataRed_heavy_non_edge_branch(int k);
    int dataRed_heavy_edge_single_end(int k);
    int dataRed_heavy_edge_single_end_branch(int k);
    int dataRed_heavy_edge_both_ends(int k);
    int dataRed_remove_existing_clique();
    void remove_clique(std::vector<int> &component);
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


    // heuristics
    bool terminate = false;
    void run_heuristic();
    void heuristic0();
    void heuristic1();
    void heuristic2();

    void localSearch();

    void random_cluster_graph();
    int clusterMove(int u, int v);
    std::vector<int> compute_vertex_cost();

    int best_k = INT32_MAX;
    std::vector<std::pair<int,int>> best_solution;
    void save_best_solution();
    void output_heuristic_solution();
    void verify_best_solution();

    void output_modified_edges();
    int compute_modified_edge_cost();

};


#endif //ALGENG_WCE_SOLVER_SOLVER_H
