//
// Created by zia on 4/21/21.
//

#include <iostream>
#include "include/cluster_graph.h"

using namespace std;

cluster_graph cg;

// O(3^log(k)*n*log(n) + log(k)*n^(2))  --  log(k) is cost of binary search, 3^log(k)*n*log(n) total cost of cg.solve, log(k)*n^2 is total cost of resetting cluster graph
int binary_search_for_optimal_k(int l, int r) {         // r is always considered to be the solution, just not always the optimal
    if(l == r) return r;

    int m = (l+r)/2;
    int ret = cg.solve(m);
    if(ret == -1)
        return binary_search_for_optimal_k(m+1, r);
    else
    {
        cg.reset_graph();                           // resetting graph to starting position after finding solution
        return binary_search_for_optimal_k(l, m);
    }
}

// O(3^log(k)*n*log(n) + log(k)*n^(2) + n^3)  -- n^3 is for calculating p3 for the first time when graph data is loaded
int main(int argc, char **args)
{
    cg.load_graph(argc == 1 ? "" : args[1]);
    int k;
    for (k = 1; cg.solve(k) == -1; k*=2);
    cg.reset_graph();

    k = binary_search_for_optimal_k(k/2+1, k);
    cg.solve(k);
//    cout << k << endl;
//    cout << "-------------------------------------------------\n";
    for (int i = 0; i < cg.n; i++)
    {
        for (int j = i + 1; j < cg.n; j++)
            if (cg.all_edge_statuses[i][j] != 2)
                cout << i + 1 << " " << j + 1 << endl;
    }
//    cout << "-------------------------------------------------\n";

    cout << "#recursive steps: " << cg.rec_steps << endl;

    return 0;
}
