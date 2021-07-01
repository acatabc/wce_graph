#include "HeuristicSolver.h"
#include "WCE_Graph.h"
#include <iostream>
#include "../include/utils.h"
#include <csignal>
#include <unistd.h>

static bool terminate = false;

HeuristicSolver::HeuristicSolver(WCE_Graph *input_graph){
    int num_vertices = input_graph->active_nodes.size();

    g = new WCE_Graph(num_vertices);
    for(int i = 0; i < input_graph->active_nodes.size(); i++){
        for(int j = i+1; j < input_graph->active_nodes.size(); j++) {
            int weight = input_graph->get_weight(input_graph->active_nodes[i], input_graph->active_nodes[j]);
            g->set_weight(i, j, weight);
            g->set_weight_original(i, j, weight);
        }

    }

    srand(time(NULL));
}


void HeuristicSolver::solve() {
    if(g->num_vertices == 0) return;
    std::signal(SIGALRM, signal_handler);

    alarm(1);


    while(!terminate){
        g->reset_graph();
        int k = greedy_cluster_graph();
        int dk = local_search();
        if(k - dk < best_k){
            save_current_solution();
            best_k = k - dk;
        }
    }
    output_best_solution();
    verify_best_solution();
}


int HeuristicSolver::compute_upper_bound() {
    if(g->num_vertices == 0) return 0;
    std::signal(SIGALRM, signal_handler);

    alarm(20);

    int num_iterations = 10;

    while(!terminate && num_iterations > 0){
        printDebug("start heuristic iteration... ");
        g->reset_graph();
        int k = greedy_cluster_graph();
        int dk = local_search();
        if(k - dk < best_k){
            save_current_solution();
            best_k = k - dk;
        }
        num_iterations --;
    }
    return best_k;
}


int HeuristicSolver::local_search() {

    int no_improvement_count = 0; // counts the number of iterations without improvement
    int dk = 0;

    while(!terminate){
        // stop local search when we had no improvement for x iterations
        if(no_improvement_count >= 1500){
            printDebug("Stop local search (no improvement for " + std::to_string(no_improvement_count) + " iterations)");
            break;
        }

        // choose random vertex and cluster
        int u = rand() % g->active_nodes.size();
        int cluster = rand() % g->active_nodes.size();

        if(cluster == u || g->get_weight(u, cluster) > 0) {
            no_improvement_count += 1;
            continue;
        }

        // move u to cluster of cluster if this results in a lower cost k
        int k = move_to_cluster(u, cluster);

        if(k < 0) {
            printDebug("improvement k: " + std::to_string(k));
            no_improvement_count = 0;
            dk += k;
        }else
            no_improvement_count += 1;
    }
    return -dk;
}

// moves vertex u to v v if this results in lower cost k
// returns the difference in k to the previous solution
int HeuristicSolver::move_to_cluster(int u, int v) {
    std::pair<std::list<int>, std::list<int>> neighborhood_u = g->closed_neighbourhood(u);

    // compute difference in cost for changing the cluster
    int dk = 0;
    for(int neigh_u : neighborhood_u.first){
        if(neigh_u == u) continue;
        // delete all neighbors of u
        int weight = g->get_weight_original(u, neigh_u);
        if(weight == DO_NOT_ADD) continue;
        if(weight > 0)
            dk += abs(weight); // add cost for deleting edge
        else
            dk -= abs(weight); // subtract cost since deleting recovers original connection

    }

    // at this point dk is the cost for making u a single cluster
    int cost_u_alone = dk;

    // compute cost for adding u to cluster v
    std::pair<std::list<int>, std::list<int>> neighborhood_v = g->closed_neighbourhood(v);
    for(int neigh_v : neighborhood_v.first){
        if(neigh_v == u) continue;
        // add u to cluster of v
        int weight = g->get_weight_original(u, neigh_v);
        if(weight == DO_NOT_ADD) { // not allowed to add u to cluster of v
            dk = 0;
            break;
        }
        if(weight < 0)
            dk += abs(weight); // add cost for adding edge
        else
            dk -= abs(weight); // subtract cost since adding recovers original connection
    }

    // it is best to make u a single cluster
    if(cost_u_alone < dk && cost_u_alone < 0){
        printDebug("found dk: " + std::to_string(dk));
        // delete all neighbors of u
        for(int neigh_u : neighborhood_u.first){
            if(neigh_u == u) continue;
            g->delete_edge(u, neigh_u);
        }
        return cost_u_alone;
    }
    // it is best to move u to cluster of v
    else if(dk < 0){
        printDebug("found dk: " + std::to_string(dk));
        // delete all neighbors of u
        for(int neigh_u : neighborhood_u.first){
            if(neigh_u == u) continue;
            g->delete_edge(u, neigh_u);
        }
        // add u to cluster of v
        for(int neigh_v : neighborhood_v.first){
            if(neigh_v == u) continue;
            g->add_edge(u, neigh_v);
        }
    }
    return dk;
}

// greedily transforms the current graph into a cluster graph
// randomly chooses a vertex and makes its neighborhood a cluster until no vertices are left
int HeuristicSolver::greedy_cluster_graph() {
    std::vector<int> vertices = g->active_nodes;

    int k = 0;
    while (vertices.size() != 0) {
        restart:
        // choose a random vertex and get its neighborhood
        int u = vertices[rand() % vertices.size()];
        std::pair<std::list<int>, std::list<int>> neighborhood = g->closed_neighbourhood(u);

        // check for DNA edges between neighbors
        for(int neigh1 : neighborhood.first) {
            for (int neigh2 : neighborhood.first) {
                if (neigh1 == neigh2) continue;
                if (g->get_weight(neigh1, neigh2) == DO_NOT_ADD) {
                    goto restart;
                }
            }
        }

        // make cluster C = {u} + N(u)
        for(int neigh1 : neighborhood.first){
            // add all connections between neighbors
            for(int neigh2 : neighborhood.first){
                if(neigh1 == neigh2) continue;
                if(g->get_weight(neigh1, neigh2) <= 0){  // 0-edge = non-edge   -->  edge must be added
                    g->add_edge(neigh1, neigh2);
                    k += abs(g->get_weight_original(neigh1, neigh2));
                }
            }
            // cut all connections between neighbors and not-neighbors
            for(int notNeigh : neighborhood.second){
                if(neigh1 == notNeigh) continue;
                if(g->get_weight(neigh1, notNeigh) > 0){  // 0-edge = non-edge   -->  edge must NOT be deleted
                    g->delete_edge(neigh1, notNeigh);
                    k += abs(g->get_weight_original(neigh1, notNeigh));
                }
            }
        }

        // delete C = {u} + N(u) from V
        // all vertices in C form a cluster and can now be disregarded
        for(int neigh : neighborhood.first){
            std::vector<int>::iterator it;
            for(it=vertices.begin(); it != vertices.end(); ++it){
                if(*it == neigh){
                    vertices.erase(it);
                    break;
                }
            }
        }
    }

    printDebug("Greedy cluster graph k: " + std::to_string(k));

    return k;
}



// saves the modified edges for the current graph in "best_solution_stack" if this results in a lower k than current best_solution_stack
void HeuristicSolver::save_current_solution(){
    int prev_sol_size = best_solution.size();

    best_solution.clear();
    int k = 0;
    for(int i = 0; i < g->num_vertices; ++i){
        for(int j = 0; j < g->num_vertices; ++j){
            if(i >= j) continue;
            if(g->get_weight(i,j) > 0  && g->get_weight_original(i,j) < 0 ){
                k += abs(g->get_weight_original(i,j));
                best_solution.push_back(std::make_pair(i,j));
            }
            if(g->get_weight(i,j) < 0  && g->get_weight_original(i,j) > 0 ){
                k += abs(g->get_weight_original(i,j));
                best_solution.push_back(std::make_pair(i,j));
            }
            // original 0 means the edge does not exist
            if(g->get_weight(i,j) > 0  && g->get_weight_original(i,j) == 0 ){
                best_solution.push_back(std::make_pair(i,j));
            }
        }
    }

    printDebug("New/Prev solution (k: " + std::to_string(k) + "/" + std::to_string(best_k) + "), (size: " + std::to_string(best_solution.size()) + "/" + std::to_string(prev_sol_size) + ")");

    return;
}


// outputs edges in "best_solution_stack"
void HeuristicSolver::output_best_solution(){
    for(auto edge: best_solution){
        std::cout << edge.first + 1 << " " << edge.second + 1 << "\n";
    }
    std::cout << "#k: " << best_k << "\n";
}


// generates graph from modified edges in best_solution_stack and verifies that this graph is a cluster graph
void HeuristicSolver::verify_best_solution(){
    g->reset_graph();

    for(auto edge: best_solution){
        if(g->get_weight_original(edge.first, edge.second) > 0) g->delete_edge(edge.first, edge.second);
        else g->add_edge(edge.first, edge.second);
    }

    g->verify_cluster_graph();

    return;
}



void HeuristicSolver::signal_handler(int signal){
    terminate = true;
    std::cout << "#ALARM" << "\n";
}
