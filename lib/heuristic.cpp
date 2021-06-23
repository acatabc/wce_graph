
#include "Solver.h"
#include "../include/utils.h"
#include <random>


void Solver::run_heuristic() {
    if(g->num_vertices > MAX_NUM_VERTICES) return;

    heuristic2();
}


void Solver::heuristic0() {
    random_cluster_graph();
    localSearch();
    output_modified_edges();
    verify_clusterGraph();
}

void Solver::heuristic1() {

    srand(time(NULL));
    g->printGraph(std::cout);

    while(!terminate){
        g->reset_graph();        // reset graph to its original
        random_cluster_graph();  // greedy cluster graph initialization
        localSearch();           // local search until minimum is reached
        save_best_solution();    // save computed solution if its better than the best one
//        verify_clusterGraph();
    }
    output_heuristic_solution();
    verify_best_solution();
}



void Solver::heuristic2() {
    std::cout << "# Heuristic 2...\n";

    srand(time(NULL));

//    int count = 0;
//    int no_improvement_count = 0;
    while(!terminate){
//        count += 1;

//        std::cout << "#Heuristic iteration " << count << "\n";
//        printDebug("\nHeuristic iteration " + std::to_string(count));

        // stop heuristic search if there is no improvement for x iterations
//        if(no_improvement_count > 10){
//            printDebug("Stop heuristic search (no improvement for " + std::to_string(no_improvement_count) + " iterations)");
//            break;
//        }
//        int old_k = best_k;

        g->reset_graph();        // reset graph to its original
        random_cluster_graph();  // greedy cluster graph initialization
        localSearch();  // local search until minimum is reached
        save_best_solution();    // save computed solution if its better than the best one

//        verify_clusterGraph();

//        if(old_k != best_k)
//            no_improvement_count = 0;
//        else
//            no_improvement_count += 1;
    }

    output_heuristic_solution();
//    verify_best_solution();
}


// see localSearch()
// pick random vertex u based on distribution of vertex costs
void Solver::localSearch() {
    std::default_random_engine generator;

    printDebug("Random cluster graph k: " + std::to_string(compute_modified_edge_cost()));

    int count = 100; // pick maximum number of iterations
    int no_improvement_count = 0; // counts the number of iterations without improvement

    while(!terminate){
        count -= 1;

        // stop local search when we had no improvement for x iterations
        if(no_improvement_count >= 1500){
            printDebug("Stop local search (no improvement for " + std::to_string(no_improvement_count) + " iterations)");
            break;
        }

//        // u is random random vertex according to distribution of costs
//        // vertices resulting in high costs are more likely to be chosen
//        std::vector<int> vertex_costs = compute_vertex_cost();
//        std::discrete_distribution<int> distribution (vertex_costs.begin(), vertex_costs.end());
//        int u = distribution(generator);

        int u = rand() % g->active_nodes.size();

        // choose any other random vertex as new cluster for u
        int v = rand() % g->active_nodes.size();

        if(v == u || g->get_weight(u,v) > 0) continue;

        // move u to cluster of v if this results in a lower cost k
        int k = clusterMove(u,v);

        printDebug(std::to_string(k));

        if(k < 0)
            no_improvement_count = 0;
        else
            no_improvement_count += 1;
    }
}

// moves vertex u to cluster v if this results in lower cost k
// returns the difference in k to the previous solution
int Solver::clusterMove(int u, int v) {
    int k = 0;

    std::pair<std::list<int>, std::list<int>> neighborhood_u = closed_neighbourhood(u);
    for(int neigh_u : neighborhood_u.first){
        if(neigh_u == u) continue;
        // delete all neighbors of u
        if(g->get_weight_original(u,neigh_u) > 0)
            k += abs(g->get_weight_original(u,neigh_u));
        else
            k -= abs(g->get_weight_original(u,neigh_u));
    }
    std::pair<std::list<int>, std::list<int>> neighborhood_v = closed_neighbourhood(v);
    for(int neigh_v : neighborhood_v.first){
        if(neigh_v == u) continue;
        // add u to cluster of v
        if(g->get_weight_original(u,neigh_v) < 0)
            k += abs(g->get_weight_original(u,neigh_v));
        else
            k -= abs(g->get_weight_original(u,neigh_v));
    }
    if(k < 0){
        printDebug("found k: " + std::to_string(k));
        for(int neigh_u : neighborhood_u.first){
            if(neigh_u == u) continue;
            // delete all neighbors of u
            g->delete_edge(u, neigh_u);
        }
        for(int neigh_v : neighborhood_v.first){
            if(neigh_v == u) continue;
            // add u to cluster of v
            g->add_edge(u, neigh_v);
        }
    }
    return k;
}

// greedily transforms the current graph into a cluster graph
// randomly chooses a vertex and makes its neighborhood a cluster until no vertices are left
void Solver::random_cluster_graph() {
    std::vector<int> vertices = g->active_nodes;
//    srand(21);

    int k = 0;
    while (vertices.size() != 0) {
        // choose a random vertex and get its neighborhood
        int u = vertices[rand() % vertices.size()];
        std::pair<std::list<int>, std::list<int>> neighborhood = closed_neighbourhood(u);

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
//            int dneigh1 = 119;
//            int dneigh2 = 101;
//            int dneigh3 = 151;
//            if(neigh == dneigh1 ||  neigh == dneigh2 || neigh == dneigh3 ){
//                printList_int(neighborhood.first);
//                printList_int(neighborhood.second);
//                std::cout << "(" << dneigh1 << "," << dneigh2 << "):" << g->get_weight(dneigh1,dneigh2) << "/" << g->get_weight_original(dneigh2,dneigh1) << "\n";
//                std::cout << "(" << dneigh2 << "," << dneigh3 << "):" << g->get_weight(dneigh2,dneigh3) << "/" << g->get_weight_original(dneigh2,dneigh3) << "\n";
//                std::cout << "(" << dneigh3 << "," << dneigh1 << "):" << g->get_weight(dneigh3,dneigh1) << "/" << g->get_weight_original(dneigh3,dneigh1) << "\n";
//                printDebug("stop");
//            }
            std::vector<int>::iterator it;
            for(it=vertices.begin(); it != vertices.end(); ++it){
                if(*it == neigh){
                    vertices.erase(it);
                    break;
                }
            }
        }
    }
}

// outputs all modified edges based on the current graph
void Solver::output_modified_edges(){
    for(int i = 0; i < g->num_vertices; ++i){
        for(int j = 0; j < g->num_vertices; ++j){
            if(i >= j) continue;
            if(g->get_weight(i,j) > 0  && g->get_weight_original(i,j) < 0 ){
                std::cout << i +1 << " " << j+1 << "\n";
            }
            if(g->get_weight(i,j) < 0  && g->get_weight_original(i,j) > 0 ){
                std::cout << i +1 << " " << j+1 << "\n";
            }
            // original 0 means the edge does not exist
            if(g->get_weight(i,j) > 0  && g->get_weight_original(i,j) == 0 ){
                std::cout << i +1 << " " << j+1 << "\n";
            }
        }
    }
}


// saves the modified edges for the current graph in "best_solution" if this results in a lower k than current best_solution
void Solver::save_best_solution(){
    std::vector<std::pair<int,int>> modified_edges = std::vector<std::pair<int,int>>();
    int k = 0;
    for(int i = 0; i < g->num_vertices; ++i){
        for(int j = 0; j < g->num_vertices; ++j){
            if(i >= j) continue;
            if(g->get_weight(i,j) > 0  && g->get_weight_original(i,j) < 0 ){
                k += abs(g->get_weight_original(i,j));
                modified_edges.push_back(std::make_pair(i,j));
            }
            if(g->get_weight(i,j) < 0  && g->get_weight_original(i,j) > 0 ){
                k += abs(g->get_weight_original(i,j));
                modified_edges.push_back(std::make_pair(i,j));
            }
            // original 0 means the edge does not exist
            if(g->get_weight(i,j) > 0  && g->get_weight_original(i,j) == 0 ){
                modified_edges.push_back(std::make_pair(i,j));
            }
        }
    }
    if (k < best_k){
        best_k = k;
        best_solution = modified_edges;
    }
    printDebug("Current/Best solution k: " + std::to_string(k) + "/" + std::to_string(best_k) + " size: " + std::to_string(modified_edges.size()) + "/" + std::to_string(best_solution.size()));
    return;
}

// outputs edges in "best_solution"
void Solver::output_heuristic_solution(){
    for(auto edge: best_solution){
        std::cout << edge.first + 1 << " " << edge.second + 1 << "\n";
    }
    std::cout << "#k: " << best_k << "\n";
}

// computes the total cost of all modified edges based on the current graph
int Solver::compute_modified_edge_cost(){
    int k = 0;
    for(int i = 0; i < g->num_vertices; ++i){
        for(int j = 0; j < g->num_vertices; ++j){
            if(i >= j) continue;
            if(g->get_weight(i,j) > 0  && g->get_weight_original(i,j) < 0 ){
                k += abs(g->get_weight_original(i,j));
            }
            if(g->get_weight(i,j) < 0  && g->get_weight_original(i,j) > 0 ){
                k += abs(g->get_weight_original(i,j));
            }
        }
    }
    return k;
}

// returns an array with "vertex-cost" for every vertex in the graph
// cost(v) = sum |weight(v,x)| for all modified edges (v,x)
std::vector<int>  Solver::compute_vertex_cost(){
    std::vector<int> vertex_cost = std::vector<int>(g->num_vertices);
    for(int i = 0; i < g->num_vertices; ++i) {
        for (int j = 0; j < g->num_vertices; ++j) {
            if (i >= j) continue;
            if (g->get_weight(i, j) > 0 && g->get_weight_original(i, j) < 0) {
                int dk = abs(g->get_weight_original(i, j));
                vertex_cost[i] += dk;
                vertex_cost[j] += dk;
            }
            if (g->get_weight(i, j) < 0 && g->get_weight_original(i, j) > 0) {
                int dk = abs(g->get_weight_original(i, j));
                vertex_cost[i] += dk;
                vertex_cost[j] += dk;
            }
        }
    }
    return vertex_cost;
}



// generates graph from modified edges in best_solution and verifies that this graph is a cluster graph
void Solver::verify_best_solution(){
    g->reset_graph();

    for(auto edge: best_solution){
        if(g->get_weight_original(edge.first, edge.second) > 0) g->delete_edge(edge.first, edge.second);
        else g->add_edge(edge.first, edge.second);
    }

    printDebug("\n#Verifying best solution...");
    auto p3 = this->get_max_cost_p3_naive();
    if(std::get<0>(p3) == -1){
        printDebug("#VERIFICATION SUCCESS\n");
    } else {
        printDebug("#VERIFICATION FAIL:");
        print_tuple(p3);
        int u = std::get<0>(p3);
        int v = std::get<1>(p3);
        int w = std::get<2>(p3);
        std::cout << "(" << u << "," << v << "):" << g->get_weight(u,v) << "/" << g->get_weight_original(u,v) << "\n";
        std::cout << "(" << v << "," << w << "):" << g->get_weight(w,v) << "/" << g->get_weight_original(w,v) << "\n";
        std::cout << "(" << u << "," << w << "):" << g->get_weight(u,w) << "/" << g->get_weight_original(u,w) << "\n";
    }

    return;
}