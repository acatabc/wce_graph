
#include <iostream>
#include <tuple>
#include "WCE_Graph.h"
#include "../include/utils.h"

WCE_Graph *parse_and_build_graph(const char* filename){
#ifdef DEBUG
    //file test_data.txt > stdin
    //I dont like the file path thing but ok...
    //std::cout <<"../test_data/test_data.txt" << std::endl;
    freopen(filename, "r", stdin);
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

void printList(std::list<std::tuple<int, int, int, int>> mylist){
#ifdef DEBUG
    if(PRINTDEBUG == true){
        std::list<std::tuple<int, int, int, int>>::iterator it;
        std::cout << "---------\n";
        std::cout << "mylist contains:\n";
        for(it=mylist.begin(); it!=mylist.end(); ++it){
            std::cout << "(" << std::get<0>(*it) << "," << std::get<1>(*it) <<"," << std::get<2>(*it) << ") cost " << std::get<3>(*it) << "\n";
//            std::cout << "(" << (std::get<0>(*it)+1) << "," << (std::get<1>(*it)+1) <<"," << (std::get<2>(*it)+1)  << ") cost " << std::get<3>(*it) << "\n";
        }
        std::cout << "list end\n";
        std::cout << "---------\n";
    }
#endif
}

void printDebug(std::string line){
#ifdef DEBUG
    if(PRINTDEBUG == true){
        std::cout << line << std::endl;
    }
#endif
}

void print_tuple(std::tuple<int,int,int> a){
#ifdef DEBUG
    std::cout << std::get<0>(a) +1<< " " << std::get<1>(a) +1<< " " << std::get<2>(a) +1<< std::endl;
#endif
}

void print_tuple4(std::tuple<int,int,int,int> a){
#ifdef DEBUG
    std::cout << std::get<0>(a)<< " " << std::get<1>(a) << " " << std::get<2>(a)<< std::get<3>(a)<< std::endl;
#endif
}