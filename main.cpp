#include <iostream>
#include <tuple>
#include "include/utils.h"
int main() {
#ifdef DEBUG
    std::cout << "Debug Option yeah" << std::endl;
#endif
    WCE_Graph *g = parse_and_build_graph();
#ifdef DEBUG
    g->print(std::cout);
    do{
        auto p = g->find_next_p3();
        std::cout << "(" << std::get<0>(p) <<", " << std::get<1>(p) << ", " << std::get<2>(p) << ")" << std::endl;
        if(std::get<0>(p) == -1)
            break;
    }while(1);

    std::cout << "++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
#endif
    g->solve();
    return 0;
}
