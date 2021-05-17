//
// Created by acat on 17.05.21.
//

#include "lib/Solver.h"

int main() {

//    WCE_Graph *g = parse_and_build_graph("../test_data/a001.dimacs");
//    Solver2 *s = new Solver2();
    Solver *s = new Solver();

    s->output_data_reduction();

    return 0;
}
