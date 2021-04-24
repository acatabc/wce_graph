//
// Created by acat on 16.04.21.
//

#ifndef ALGENG_WCE_SOLVER_UTILS_H
#define ALGENG_WCE_SOLVER_UTILS_H

#include <iostream>
#include <list>

#define PRINTDEBUG true

void print_tuple4(std::tuple<int,int,int,int> a){
    std::cout << std::get<0>(a)<< " " << std::get<1>(a) << " " << std::get<2>(a)<< std::get<3>(a)<< std::endl;
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
    std::cout << std::get<0>(a) +1<< " " << std::get<1>(a) +1<< " " << std::get<2>(a) +1<< std::endl;
}


#endif //ALGENG_WCE_SOLVER_UTILS_H


