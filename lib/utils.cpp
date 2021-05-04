//
// Created by acat on 16.04.21.
//
#include <iostream>
#include <tuple>
#include "WCE_Graph.h"


void print_tuple(std::tuple<int,int,int> a){
    std::cout << std::get<0>(a) +1<< " " << std::get<1>(a) +1<< " " << std::get<2>(a) +1<< std::endl;
}
