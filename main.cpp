#include "lib/Solver.h"

int main() {
    Solver *s = new Solver();
//    s->solve();
    auto neighbours = s->closed_neighbourhood(1);
    s->min_cut(neighbours.first);
    return 0;
}
