#include <iostream>
#include <tuple>
#include "include/utils.h"
int main() {

    WCE_Graph *g = parse_and_build_graph();
#ifdef DEBUG
    g->print_all_p3();
#endif
    g->solve();
    return 0;
}
