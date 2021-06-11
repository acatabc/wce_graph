#include "lib/Solver.h"
#include <csignal>
#include <unistd.h>

#include <iostream>


Solver *s;

void signal_handler(int signal){
    s->terminate = true;
}

int main() {
    std::signal(SIGALRM, signal_handler);

    alarm(8);

    s = new Solver();
    s->run_heurisic();
//    s->solve();
    return 0;
}
