#include "lib/Solver.h"

int main() {
    Solver *s = new Solver();
    s->solve();
//    auto neighbours = s->closed_neighbourhood(2);
//    s->min_cut(neighbours.first);
    return 0;
}
