
#ifndef ALGENG_WCE_SOLVER_UTILS_H
#define ALGENG_WCE_SOLVER_UTILS_H

#include <iostream>
#include <list>

#define DO_NOT_DELETE INT32_MAX
#define DO_NOT_ADD INT32_MIN



#define MAX_NUM_VERTICES 1000
#define PRINTDEBUG true


//const char* FILENAME = "../wce-students/specialTests/w27.dimacs";
#define FILENAME  "../wce-students/2-real-world/w005.dimacs"
//const char* FILENAME = "../../wce-students-real/2-real-world/w061.dimacs";
//const char* FILENAME = "../test_data/w001.dimacs";



void printDebug(std::string line);

void print_tuple(std::tuple<int,int,int> a);

void print_tuple4(std::tuple<int,int,int,int> a);

void printList(std::list<std::tuple<int, int, int, int>> mylist);

void printList_int(std::list<int> mylist);

void printVector_int(std::vector<int> myVec);


void throwError(std::string msg);


WCE_Graph *parse_and_build_graph();

#endif //ALGENG_WCE_SOLVER_UTILS_H


