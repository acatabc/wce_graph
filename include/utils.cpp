

#include <iostream>
#include <tuple>
#include "../lib/WCE_Graph.h"
#include "utils.h"

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


void printList_int(std::list<int> mylist){
#ifdef DEBUG
    if(PRINTDEBUG == true){
        std::list<int>::iterator it;
        std::cout << "---------\n";
        std::cout << "mylist contains:\n";
        for(it=mylist.begin(); it!=mylist.end(); ++it){
            std::cout << *it << "," ;
        }
        std::cout << "---------\n";
    }
#endif
}


void printVector_int(std::vector<int> myVec){
#ifdef DEBUG
    if(PRINTDEBUG == true){
//        std::cout << "---------\n";
        std::cout << "vec: [ ";
        for(int i = 0; i < myVec.size(); ++i){
            if(i != myVec.size()-1)
                std::cout << myVec[i] << ", " ;
            else
                std::cout << myVec[i];
        }
        std::cout <<"]\n";
//        std::cout << "---------\n";
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
    std::cout << std::get<0>(a)<< " " << std::get<1>(a)<< " " << std::get<2>(a)<< std::endl;
#endif
}

void print_tuple4(std::tuple<int,int,int,int> a){
#ifdef DEBUG
    std::cout << std::get<0>(a)<< " " << std::get<1>(a) << " " << std::get<2>(a)<< std::get<3>(a)<< std::endl;
#endif
}


void throwError(std::string text){
//#ifdef DEBUG
    throw std::invalid_argument( text );
//#endif
}




WCE_Graph *parse_and_build_graph(){

#ifdef DEBUG
    //    freopen("../wce-students/2-real-world/w027.dimacs", "r", stdin);
//    freopen("../test_data/r049.dimacs", "r", stdin);
    auto x = freopen(FILENAME, "r", stdin);
    if (!x) printDebug("Cannot open file");
#endif
    int num_vertices = 0;
    std::cin >> num_vertices;

    if(num_vertices > MAX_NUM_VERTICES){  // greedy-greedy: output all positive edges if too many vertices in input graph
        int v, w, weight;
        while(std::cin) {
            std::cin >> v >> w >> weight;
            if(weight > 0) std::cout << v << " " << w << "\n";
        }
        return new WCE_Graph(num_vertices);
    }

    WCE_Graph *g = new WCE_Graph(num_vertices);

    int v, w, weight;
    while(std::cin){
        std::cin >> v >> w >> weight;
        v -= 1;
        w -= 1;
        if(!std::cin.fail()) {
            g->set_weight(v, w, weight);
            g->set_weight_original(v, w, weight);
        }
    }
    return g;
}
