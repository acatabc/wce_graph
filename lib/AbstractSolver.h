

#ifndef ALGENG_WCE_SOLVER_ABSTRACTSOLVER_H
#define ALGENG_WCE_SOLVER_ABSTRACTSOLVER_H


#include <vector>
#include <map>
#include "WCE_Graph.h"


#define NONE (-1)
#define CLUSTER_GRAPH (-2)

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



    // data reduction
    virtual int data_reduction(int k, int layer) = 0;
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
};


#endif //ALGENG_WCE_SOLVER_ABSTRACTSOLVER_H
