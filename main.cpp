#include <iostream>
#include <tuple>
#include "include/utils.h"
int main() {

    Solver *s = new Solver();


#ifdef DEBUG
    s->g->print(std::cout);
    s->get_all_p3();
    int i = 0;
    for(auto a : s->p3s){
        for(auto b: a){
//            std::cout << std::get<0>(b.first) << " with " << i+1 <<" " << b.second.first+1 << " " << b.second.second +1<< std::endl;
        }
        ++i;
    }
#endif
    s->solve();
    return 0;
}
