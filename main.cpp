#include <iostream>
#include <tuple>
#include "include/utils.h"
int main() {

    WCE_Graph *g = parse_and_build_graph("../test_data/test_data.txt");
    Solver *s = new Solver(g);
    g->printGraph(std::cout);

#ifdef DEBUG
    s->print_all_p3();
#endif
    s->solve();
    return 0;
}
