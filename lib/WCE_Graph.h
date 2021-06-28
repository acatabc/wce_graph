
#ifndef ALGENG_WCE_SOLVER_WCE_GRAPH_H
#define ALGENG_WCE_SOLVER_WCE_GRAPH_H
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>
#include <list>
#include <vector>
#include <stack>

// type of operations in graph modification stack
#define MERGE 1
#define SET_INF 2
#define CLIQUE 3


class WCE_Graph {
private:
    struct matrix_entry{
        int weight;
        bool valid_entry;
        int weight_original;
    };
    std::vector<std::vector<matrix_entry>> adj_matrix;
    void unmerge(int);

public:
    WCE_Graph(int);
    const int num_vertices;
    virtual ~WCE_Graph();

    // edges
    int get_weight(int, int);
    void set_weight(int, int, int);

    int get_weight_original(int, int);
    void set_weight_original(int, int, int);

    void add_edge(int,int);
    void delete_edge(int, int);

    // p3s
    int get_cost(int u, int v, int w);

    // merging
    std::vector<int> active_nodes;
    std::vector<std::vector<int>> merge_map;
    int merge(int, int);
    void set_non_edge(int u, int v);
    void remove_clique(std::vector<int> &component);
    void undo_final_modification();

    struct stack_elem{
        int type; // 1 is merge, 2 is set_inf, 3 is clique
        int v1; // edge set_inf
        int v2;
        int weight;
        int uv; // merged vertex
        std::vector<int> clique; // remove clique
    };
    std::stack<stack_elem> graph_mod_stack;


    void DFS(int , bool *, std::vector<int>&);
    std::pair<std::list<int>, std::list<int>> closed_neighbourhood(int u);
    int deficiency(std::list<int> neighbours);
    int cut_weight(std::list<int>& neighbourhood, std::list<int>& rest_graph);

    void printGraph(std::ostream &os);
    void print_active_graph(std::ostream &os);
    void print_active_nodes();
    void print_merge_map();
    void print_graph_mod_stack();
    void print_graph_mod_stack_rec();

    // heuristics
    void reset_graph();
};

#endif //ALGENG_WCE_SOLVER_WCE_GRAPH_H
