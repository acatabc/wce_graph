#include <iostream>
#include "include/utils.h"
int main() {
#ifdef DEBUG
    std::cout << "Debug Option yeah" << std::endl;
#endif
    WCE_Graph *g = parse_and_build_graph();
    return 0;
}
