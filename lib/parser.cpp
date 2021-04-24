//
// Created by acat on 16.04.21.
//
#include <iostream>
#include <tuple>
#include "WCE_Graph.h"

WCE_Graph *parse_and_build_graph(){
#ifdef DEBUG
    //file test_data.txt > stdin
    //I dont like the file path thing but ok...
    //std::cout <<"../test_data/test_data.txt" << std::endl;
    freopen("../test_data/a001.dimacs", "r", stdin);
#endif
    int num_vertices = 0;
    std::cin >> num_vertices;
    WCE_Graph *g = new WCE_Graph(num_vertices);
    int v, w, weight;
    while(std::cin){
        std::cin >> v >> w >> weight;
        v -= 1;
        w -= 1;
        if(!std::cin.fail())
            g->set_weight(v,w, weight);
    }
    return g;
}

