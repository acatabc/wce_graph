
#ifndef ALGENG_WCE_SOLVER_UTILS_H
#define ALGENG_WCE_SOLVER_UTILS_H

#include <iostream>
#include <list>

#define DO_NOT_DELETE INT32_MAX
#define DO_NOT_ADD INT32_MIN

#define PRINTDEBUG true


void printList(std::list<std::tuple<int, int, int, int>> mylist);

void printDebug(std::string line);

void print_tuple(std::tuple<int,int,int> a);

void print_tuple4(std::tuple<int,int,int,int> a);


#endif //ALGENG_WCE_SOLVER_UTILS_H


