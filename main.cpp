
#include "lib/Solver.h"
#include "lib/Solver2.h"

int main() {

//    WCE_Graph *g = parse_and_build_graph("../test_data/a001.dimacs");
    Solver2 *s = new Solver2();
//    Solver *s = new Solver(g);

//    g->printGraph(std::cout);

    s->solve();

    return 0;
}
