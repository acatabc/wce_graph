

#include "lib/Solver.h"
#include "lib/Solver2.h"

int main() {


//    WCE_Graph *g = parse_and_build_graph("../test_data/a001.dimacs");
//    Solver2 *s = new Solver2();
    Solver *s = new Solver();


//    g->printGraph(std::cout);
//    s->dataRed_remove_existing_clique();

    s->solve();

    return 0;
}
