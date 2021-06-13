#include "lib/Solver.h"
#include <csignal>
#include <unistd.h>

#include <iostream>


Solver *s;

void signal_handler(int signal){
    s->terminate = true;
    std::cout << "#ALARM" << "\n";
}

int main() {
    std::signal(SIGALRM, signal_handler);

    alarm(59);

    s = new Solver();
    s->run_heuristic();
//    s->solve();
    return 0;
}
