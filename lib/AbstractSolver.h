

#ifndef ALGENG_WCE_SOLVER_ABSTRACTSOLVER_H
#define ALGENG_WCE_SOLVER_ABSTRACTSOLVER_H


#include <vector>
#include <map>
#include "WCE_Graph.h"

#define NONE (-1)
#define CLUSTER_GRAPH (-2)


#define MAX_SUM_P3 0
#define MAX_MIN_EDGE_P3 1
#define LOWER_BOUND_FAST 0
#define LOWER_BOUND_IMPROVED 1



class AbstractSolver {

private:
public:
    AbstractSolver(WCE_Graph *);
    virtual ~AbstractSolver();
    WCE_Graph *g;

    virtual void solve() = 0;
    virtual int branch(int k, int layer) = 0;
    unsigned int rec_steps = 0;

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

    int dataRed_weight_k(int k);
    int dataRed_heavy_non_edge(int k);
    int dataRed_heavy_edge_single_end(int k);
    int dataRed_heavy_edge_both_ends(int k);
    int dataRed_remove_clique();
    int dataRed_large_neighbourhood_I(int);

    // output
    int unmerge_and_propagate(int uv);
    void clear_stack_and_output();
    void final_output(int u, int v);
};


#endif //ALGENG_WCE_SOLVER_ABSTRACTSOLVER_H
