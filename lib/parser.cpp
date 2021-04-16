//
// Created by acat on 16.04.21.
//
#include <iostream>
#include "WCE_Graph.h"
#include "../include/personal_settings.h"

WCE_Graph *parse_and_build_graph(){
#ifdef DEBUG
    //file test_data.txt > stdin
    std::cout <<PATH_TO_TEST_DATA"/test_data.txt" << std::endl;
    freopen(PATH_TO_TEST_DATA"/test_data.txt", "r", stdin);
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
