
#ifndef ALGENG_WCE_SOLVER_SOLVER_H
#define ALGENG_WCE_SOLVER_SOLVER_H

#include <vector>
#include <map>
#include "WCE_Graph.h"


#define MAX_SUM_P3 0
#define MAX_MIN_EDGE_P3 1
#define LOWER_BOUND_FAST 0
#define LOWER_BOUND_IMPROVED 1

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

    //method related to p3s
    struct p3{
        int i,j,k;
        int cost_sum;
        int min_cost;
    };
    struct p3 generate_p3_struct(int,int,int);
    p3 get_max_cost_p3();
    std::tuple<p3, int> get_best_p3_and_lower_bound(int heuristic, int version);
    std::vector<p3> find_all_p3();


    // data reduction
    int data_reduction(int k, int layer);
    int data_reduction_before_branching();

    int dataRed_weight_larger_k(int k);
    int dataRed_heavy_non_edge(int k);
    int dataRed_heavy_edge_single_end(int k);
    int dataRed_heavy_edge_both_ends(int k);
    int dataRed_remove_existing_clique();
    int dataRed_large_neighbourhood_I(int);


    // output
    int unmerge_and_propagate(int uv);
    void clear_stack_and_output();
    void final_output(int u, int v);


    // debug
    void verify_cluster_graph();


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
