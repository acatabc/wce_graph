//
// Created by acat on 16.04.21.
//
#include <iostream>
#include "WCE_Graph.h"

WCE_Graph *parse_and_build_graph(){
#ifdef DEBUG
    //file test_data.txt > stdin
    //I dont like the file path thing but ok...
    std::cout <<"../test_data/test_data.txt" << std::endl;
    freopen("../test_data/test_data.txt", "r", stdin);
#endif
    //JUST TRYING SOME STUFF FOR SETUP
    std::string line;
    int i = 0;
    while(std::cin){
        getline(std::cin, line );
        std::cout <<i << " " << line << std::endl;
        ++i;
    }
}
