//
// Created by acat on 16.04.21.
//

#ifndef ALGENG_WCE_SOLVER_WCE_GRAPH_H
#define ALGENG_WCE_SOLVER_WCE_GRAPH_H
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>
#include <list>


class WCE_Graph {
private:
    int **adj_matrix;

public:
    WCE_Graph(int);
    int num_vertices;
    void add_edge(int,int);
    void modify_edge(int, int);
    void set_weight(int , int , int );
    int get_weight(int,int);
    int branch(int  );
    std::tuple<int, int, int> find_next_p3();
    std::tuple<int, int, int> find_first_p3();
    std::list<std::tuple<int,int,int>>get_all_p3();
    void solve();
    virtual ~WCE_Graph();

    void print(std::ostream &os);
    void print_all_p3();

    std::tuple<int, int, int> get_max_cost_p3_experimental(int *);

    std::tuple<int, int, int> get_max_cost_p3(int *costs);
};


#endif //ALGENG_WCE_SOLVER_WCE_GRAPH_H
