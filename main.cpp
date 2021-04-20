#include <iostream>
#include <tuple>
#include "include/utils.h"
int main() {

    WCE_Graph *g = parse_and_build_graph("../test_data/a001.dimacs");
    Solver *s = new Solver(g);


#ifdef DEBUG
    g->printGraph(std::cout);
    s->print_all_p3();
#endif
    s->solve();
    return 0;
}
