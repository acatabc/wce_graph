
#ifndef ALGENG_WCE_SOLVER_WCE_GRAPH_H
#define ALGENG_WCE_SOLVER_WCE_GRAPH_H
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>
#include <list>
#include <vector>
#include <stack>


class WCE_Graph {
private:
    struct matrix_entry{
        int weight;
        bool flag;
    };
    std::vector<std::vector<matrix_entry>> adj_matrix;
    void unmerge(int);

public:
    WCE_Graph(int);
    const int num_vertices;
    int num_vertices_after_reduction;
    virtual ~WCE_Graph();

    void set_weight(int, int, int);
    int get_weight(int, int);

    void add_edge(int,int);
    void delete_edge(int, int);
    void modify_edge(int, int); // old, jusst used in solver1()

    int get_cost(int u, int v, int w);

    // merging
    std::vector<int> active_nodes;
    std::vector<std::vector<int>> merge_map;
    int merge(int, int);
    void set_non_edge(int u, int v);
    void recover_original(int);
    void undo_final_modification();

    struct stack_elem{
        int type; // 1 is merge, 2 is set_inf
        // edge set_inf
        int v1;
        int v2;
        int weight;
        // merged vertex
        int uv;
    };
    std::stack<stack_elem> graph_mod_stack;

    void printGraph(std::ostream &os);
    void print_active_graph(std::ostream &os);
    void print_active_nodes();
    void print_merge_map();

    void print_stack(std::stack<stack_elem>);
    void print_stack_rec(std::stack<stack_elem>);
};

#endif //ALGENG_WCE_SOLVER_WCE_GRAPH_H
