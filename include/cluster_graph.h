//
// Created by zia on 4/9/21.
//

#ifndef CLUSTER_GRAPH_H
#define CLUSTER_GRAPH_H

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <sstream>

using namespace std;

// converts a graph into a cluster graph with given value of cost if possible
class cluster_graph
{
public:
    unsigned int rec_steps = 0;
private:
    class node
    {
    public:
        class node_weight_pair
        {
        public:
            int node_index;
            int weight;

            node_weight_pair() : node_index(-1), weight(0) {}

            node_weight_pair(int _node_index, int _weight) : node_index(_node_index), weight(_weight) {}

            friend bool operator<(const node_weight_pair &p1, const node_weight_pair &p2)
            {
                return p1.node_index < p2.node_index;
            }
        };

        set<node_weight_pair> connected_nodes;
        set<node_weight_pair> disconnected_nodes;

        node() {}
    };

    class p3
    {
    private:
        static int n;

    public:
        int u, w;        // u is always assumed to be smaller than w in comparing calculations
        p3(int u, int w) : u(u), w(w)
        {
            if (u == w)
                exit(-1);
        }

        friend bool operator<(const p3 &p3_1, const p3 &p3_2)
        {
            return
                    n * (p3_1.u < p3_1.w ? p3_1.u : p3_1.w) +
                    (p3_1.u > p3_1.w ? p3_1.u : p3_1.w)
                    <
                    n * (p3_2.u < p3_2.w ? p3_2.u : p3_2.w) +
                    (p3_2.u > p3_2.w ? p3_2.u : p3_2.w);
        }

        friend ostream &operator<<(ostream &os, const p3 &_p3)
        {
            os << "[p3: " << _p3.u + 1 << ", " << _p3.w + 1 << "]";
            return os;
        }

        static void set_n(int _n)
        {
            n = _n;
        }
    };

    node *all_nodes;
    set<p3> *all_p3;

    bool are_nodes_connected(int node_index_1, int node_index_2)
    {
        if (all_nodes[node_index_1].connected_nodes.find(node::node_weight_pair(node_index_2, 0)) !=
            all_nodes[node_index_1].connected_nodes.end())
            return true;
        return false;
    }

    bool are_nodes_disconnected(int node_index_1, int node_index_2)
    {
        if (all_nodes[node_index_1].disconnected_nodes.find(node::node_weight_pair(node_index_2, 0)) !=
            all_nodes[node_index_1].disconnected_nodes.end())
            return true;
        return false;
    }

    void disconnect_nodes(int u_index, int v_index)
    {
        node *u_node = &all_nodes[u_index], *v_node = &all_nodes[v_index];
        set<node::node_weight_pair>::iterator v_node_weight_pair_iterator = u_node->connected_nodes.find(
                node::node_weight_pair(v_index, 0));
        set<node::node_weight_pair>::iterator u_node_weight_pair_iterator = v_node->connected_nodes.find(
                node::node_weight_pair(u_index, 0));
        node::node_weight_pair v_node_weight_pair = *v_node_weight_pair_iterator;
        node::node_weight_pair u_node_weight_pair = *u_node_weight_pair_iterator;
        u_node->connected_nodes.erase(v_node_weight_pair_iterator);
        v_node->connected_nodes.erase(u_node_weight_pair_iterator);
        u_node_weight_pair.weight *= -1;
        v_node_weight_pair.weight *= -1;
        u_node->disconnected_nodes.insert(v_node_weight_pair);
        v_node->disconnected_nodes.insert(u_node_weight_pair);

        set<node::node_weight_pair>::iterator iterator1 = u_node->connected_nodes.begin();
        set<node::node_weight_pair>::iterator iterator2 = v_node->connected_nodes.begin();

        while (iterator1 != u_node->connected_nodes.end() || iterator2 != v_node->connected_nodes.end())
        {
            if (iterator1 == u_node->connected_nodes.end())
            {
                all_p3[v_index].erase(all_p3[v_index].find(p3(u_index, iterator2->node_index)));
                iterator2++;
            } else if (iterator2 == v_node->connected_nodes.end())
            {
                all_p3[u_index].erase(all_p3[u_index].find(p3(iterator1->node_index, v_index)));
                iterator1++;
            } else if (iterator1->node_index == iterator2->node_index)
            {
                all_p3[iterator1->node_index].insert(p3(u_index, v_index));
                iterator1++;
                iterator2++;
            } else if (iterator1->node_index < iterator2->node_index)
            {
                all_p3[u_index].erase(all_p3[u_index].find(p3(iterator1->node_index, v_index)));
                iterator1++;
            } else if (iterator1->node_index > iterator2->node_index)
            {
                all_p3[v_index].erase(all_p3[v_index].find(p3(u_index, iterator2->node_index)));
                iterator2++;
            }
        }
    }

    int connect_nodes(int u_index, int v_index)
    {
        node *u_node = &all_nodes[u_index], *v_node = &all_nodes[v_index];
        set<node::node_weight_pair>::iterator v_node_weight_pair_iterator = u_node->disconnected_nodes.find(
                node::node_weight_pair(v_index, 0));
        set<node::node_weight_pair>::iterator u_node_weight_pair_iterator = v_node->disconnected_nodes.find(
                node::node_weight_pair(u_index, 0));
        node::node_weight_pair v_node_weight_pair = *v_node_weight_pair_iterator;
        node::node_weight_pair u_node_weight_pair = *u_node_weight_pair_iterator;
        u_node->disconnected_nodes.erase(v_node_weight_pair_iterator);
        v_node->disconnected_nodes.erase(u_node_weight_pair_iterator);
        u_node_weight_pair.weight *= -1;
        v_node_weight_pair.weight *= -1;
        u_node->connected_nodes.insert(v_node_weight_pair);
        v_node->connected_nodes.insert(u_node_weight_pair);

        set<node::node_weight_pair>::iterator iterator1 = u_node->connected_nodes.begin();
        set<node::node_weight_pair>::iterator iterator2 = v_node->connected_nodes.begin();
        while (iterator1 != u_node->connected_nodes.end() || iterator2 != v_node->connected_nodes.end())
        {
            if (iterator1 == u_node->connected_nodes.end())
            {
                if (iterator2->node_index != u_index)
                    all_p3[v_index].insert(p3(u_index, iterator2->node_index));
                iterator2++;
            } else if (iterator2 == v_node->connected_nodes.end())
            {
                if (iterator1->node_index != v_index)
                    all_p3[u_index].insert(p3(iterator1->node_index, v_index));
                iterator1++;
            } else if (iterator1->node_index == iterator2->node_index)
            {
                all_p3[iterator1->node_index].erase(all_p3[iterator1->node_index].find(p3(u_index, v_index)));
                iterator1++;
                iterator2++;
            } else if (iterator1->node_index < iterator2->node_index)
            {
                if (iterator1->node_index != v_index)
                    all_p3[u_index].insert(p3(iterator1->node_index, v_index));
                iterator1++;
            } else if (iterator1->node_index > iterator2->node_index)
            {
                if (iterator2->node_index != u_index)
                    all_p3[v_index].insert(p3(u_index, iterator2->node_index));
                iterator2++;
            }
        }
    }

public:
    enum edge_status
    {
        ADDED, REMOVED, UNCHANGED
    };

    int n;
    edge_status **all_edge_statuses;

    cluster_graph() : n(-1), all_nodes(NULL), all_p3(NULL), all_edge_statuses(NULL) {}

    void load_graph(string file_name)     // file_name is "" if asked to read from cin
    {
        istream *input_stream = NULL;
        ifstream fin(file_name);
        if (file_name != "")
            input_stream = dynamic_cast<istream *>(&fin);
        else
            input_stream = dynamic_cast<istream *>(&cin);

        *input_stream >> n;

        p3::set_n(n);
        if (all_nodes != NULL) delete all_nodes;     // delete old nodes;
        all_nodes = new node[n];
        if (all_p3 != NULL) delete all_p3;
        all_p3 = new set<p3>[n];

        string line;
        int u, v, w;
        for (int i = 0; i < ((n - 1) * n) / 2;)
        {
            if (input_stream != &cin && input_stream->eof()) exit(-1);      // if reading from file check eof

            getline(*input_stream, line);

            if (line != "")
            {
                stringstream ss(line);
                ss >> u >> v >> w;
                if (w > 0)
                {
                    all_nodes[u - 1].connected_nodes.insert(node::node_weight_pair(v - 1, w));
                    all_nodes[v - 1].connected_nodes.insert(node::node_weight_pair(u - 1, w));
                } else if (w < 0)
                {
                    all_nodes[u - 1].disconnected_nodes.insert(node::node_weight_pair(v - 1, w));
                    all_nodes[v - 1].disconnected_nodes.insert(node::node_weight_pair(u - 1, w));
                }
                i++;
            }
        }

        if (file_name != "")
        {
            fin.close();
            input_stream = NULL;
        }

        node *v_node;
        set<node::node_weight_pair>::iterator it1, it2;
        for (int i = 0; i < n; i++)
        {
            v_node = &all_nodes[i];
            for (it1 = v_node->connected_nodes.begin(); it1 != v_node->connected_nodes.end(); it1++)
            {
                it2 = it1;
                it2++;
                for (; it2 != v_node->connected_nodes.end(); it2++)
                    if (are_nodes_disconnected(it1->node_index, it2->node_index))
                        all_p3[i].insert(p3(it1->node_index, it2->node_index));
            }
        }

        all_edge_statuses = new edge_status *[n];
        for (int i = 0; i < n; i++)
            all_edge_statuses[i] = new edge_status[n];

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                all_edge_statuses[i][j] = UNCHANGED;
    }

    // O(3^k*n*log(n) + n^3), k is budget, n is no. of nodes.
    int solve(int budget)
    {
        rec_steps++;
        if (budget < 0) return -1;

        int _u_index, _v_index, _w_index;       // for calculation
        int u_index, v_index = -1, w_index;     // for storing best p3 with max average weight
        double _max_budget_reduced;             // for calculation
        double max_budget_reduced = -1;         // for storing best p3's average weight
        for (int i = 0; i < n; i++)
        {            // search for p3 and solve for it
            _v_index = i;
            if (!all_p3[_v_index].empty())
                for (set<p3>::iterator it = all_p3[_v_index].begin(); it != all_p3[_v_index].end(); it++)
                {
                    _u_index = it->u;
                    _w_index = it->w;
                    _max_budget_reduced =
                            (all_nodes[_v_index].connected_nodes.find(node::node_weight_pair(_u_index, 0))->weight +
                             all_nodes[_v_index].connected_nodes.find(node::node_weight_pair(_w_index, 0))->weight +
                             abs(all_nodes[_u_index].disconnected_nodes.find(
                                     node::node_weight_pair(_w_index, 0))->weight)) / 3.0;
                    if (max_budget_reduced < _max_budget_reduced)
                    {
                        max_budget_reduced = _max_budget_reduced;
                        u_index = _u_index;
                        v_index = _v_index;
                        w_index = _w_index;
                    }
                }
        }

        if (v_index == -1) return budget;       // solved, solution is in all_edges_statuses

        if (all_edge_statuses[v_index][u_index] == UNCHANGED)
        {
            disconnect_nodes(v_index, u_index);
            all_edge_statuses[v_index][u_index] = REMOVED;
            all_edge_statuses[u_index][v_index] = REMOVED;

            int budget_left = budget - abs(all_nodes[v_index].disconnected_nodes.find(
                    node::node_weight_pair(u_index, 0))->weight);
            budget_left = solve(budget_left);
            if (budget_left != -1) return budget_left;

            connect_nodes(v_index, u_index);
            all_edge_statuses[v_index][u_index] = UNCHANGED;
            all_edge_statuses[u_index][v_index] = UNCHANGED;
        }
        if (all_edge_statuses[v_index][w_index] == UNCHANGED)
        {
            disconnect_nodes(v_index, w_index);
            all_edge_statuses[v_index][w_index] = REMOVED;
            all_edge_statuses[w_index][v_index] = REMOVED;

            int budget_left = budget - abs(all_nodes[v_index].disconnected_nodes.find(
                    node::node_weight_pair(w_index, 0))->weight);
            budget_left = solve(budget_left);
            if (budget_left != -1) return budget_left;

            connect_nodes(v_index, w_index);
            all_edge_statuses[v_index][w_index] = UNCHANGED;
            all_edge_statuses[w_index][v_index] = UNCHANGED;
        }
        if (all_edge_statuses[u_index][w_index] == UNCHANGED)
        {
            connect_nodes(u_index, w_index);
            all_edge_statuses[u_index][w_index] = ADDED;
            all_edge_statuses[w_index][u_index] = ADDED;

            int budget_left = budget - abs(all_nodes[u_index].connected_nodes.find(
                    node::node_weight_pair(w_index, 0))->weight);
            budget_left = solve(budget_left);
            if (budget_left != -1) return budget_left;

            disconnect_nodes(u_index, w_index);
            all_edge_statuses[u_index][w_index] = UNCHANGED;
            all_edge_statuses[w_index][u_index] = UNCHANGED;
        }
        return -1;
    }
};

int cluster_graph::p3::n = 0;

#endif