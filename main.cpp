#include "lib/Solver.h"
#include "lib/HeuristicSolver.h"
#include "lib/DeepSolver.h"
#include "include/utils.h"


int main() {

    WCE_Graph *g = parse_and_build_graph();

//    Solver *s = new Solver(g);
//    DeepSolver *d = new DeepSolver(g);
    HeuristicSolver *h = new HeuristicSolver(g);

    h->solve();
    return 0;
}

