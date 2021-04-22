#include <iostream>
#include <tuple>
#include "include/utils.h"
int main() {

    WCE_Graph *g = parse_and_build_graph();
    Solver *s = new Solver(g);


#ifdef DEBUG
//    g->print(std::cout);
//    s->print_all_p3();
#endif
    s->solve2();

    return 0;
}
