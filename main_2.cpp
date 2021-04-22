//
// Created by zia on 4/21/21.
//

#include "include/cluster_graph.h"

int main(int argc, char** args) {
    cluster_graph cg;
    cg.load_graph(argc == 1 ? "" : args[1]);

    for (int k = 1; true; k++)
    {
        cg.rec_steps = 0;
        int ret = cg.solve(k);
        if (ret != -1)
        {
//            cout << k << endl;
//            cout << "-------------------------------------------------\n";
            for (int i = 0; i < cg.n; i++)
            {
                for (int j = i + 1; j < cg.n; j++)
                    if (cg.all_edge_statuses[i][j] != 2)
                        cout << i + 1 << " " << j + 1 << endl;
            }
//            cout << "-------------------------------------------------\n";
            break;
        }
    }
    std::cout << "#recursive steps: " << cg.rec_steps << std::endl;

    return 0;
}