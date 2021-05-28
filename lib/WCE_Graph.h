
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
#define COMPONENTS 3
#define CLIQUE 4


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
    virtual ~WCE_Graph();

    void set_weight(int, int, int);
    int get_weight(int, int);

    void add_edge(int,int);
    void delete_edge(int, int);

    int get_cost(int u, int v, int w);

    // merging
    std::vector<std::vector<int>> active_nodes; // a vector of sets of active nodes (each entry is one component)
    std::vector<int> components_map; // maps each vertex to the index of its component
    std::vector<int> components_active_map; // for ech component states if it is acTiev
    std::vector<std::vector<int>> merge_map;
    int merge(int, int);
    void set_non_edge(int u, int v);
    void split_component(std::vector<std::vector<int>> components);
    void unify_components(std::vector<int> component_indices, int prev_stack_size);
    void undo_final_modification();

    struct stack_elem{
        int type; // 1 is merge, 2 is set_inf, 3 is clique
        int v1; // edge set_inf
        int v2;
        int weight;
        int uv; // merged vertex
        std::vector<int> components; // split components
        int stack_size_before_components;
        int clique; // idx of the component which is a clique
    };
    std::stack<stack_elem> graph_mod_stack;

    void printGraph(std::ostream &os);
    void print_active_graph(std::ostream &os);
    void print_active_nodes();
    void print_merge_map();
    void print_graph_mod_stack();
    void print_graph_mod_stack_rec();

    // old methods used in solver2
    void recover_original(int);
    void modify_edge(int, int);
};

#endif //ALGENG_WCE_SOLVER_WCE_GRAPH_H
