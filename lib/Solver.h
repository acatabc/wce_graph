
#ifndef ALGENG_WCE_SOLVER_SOLVER_H
#define ALGENG_WCE_SOLVER_SOLVER_H

#include <vector>
#include <map>
#include "WCE_Graph.h"
#include "AbstractSolver.h"


class Solver : public AbstractSolver {
public:
    Solver(WCE_Graph *);

    void solve();
    int branch(int k, int layer);

};


#endif //ALGENG_WCE_SOLVER_SOLVER_H
