
#include "AbstractSolver.h"
#include "../include/utils.h"


// --------------------------------
// ------- data reduction  --------

int AbstractSolver::data_reduction(int k, int layer){
    int k_before = k;

//    if(layer %5 ==  0 && layer >= 10){
//      k = dataRed_heavy_non_edge(k);
//      k = dataRed_heavy_edge_single_end(k);
//    }

    k = dataRed_weight_k(k);
    k = dataRed_heavy_edge_both_ends(k);
    k = dataRed_large_neighbourhood_I(k);
    dataRed_remove_clique();
    if(k == -1) return -1;

    return k_before - k;
}

// applies data reduction on current graph and data reduction cost (only used before branching)
int AbstractSolver::data_reduction_before_branching(){
    int k_tmp = INT32_MAX;
    int k_before = 0;
    dataRed_remove_clique();
    while(k_tmp != k_before){
        k_before = k_tmp;
        k_tmp = dataRed_heavy_non_edge(k_tmp);
        k_tmp = dataRed_heavy_edge_single_end(k_tmp);
        k_tmp = dataRed_heavy_edge_both_ends(k_tmp);
        k_tmp = dataRed_large_neighbourhood_I(k_tmp);
    }
    int cost = INT32_MAX - k_tmp;
    return cost;
}


// continuously merges all vertices whose edge weight exceeds the available costs (set edges = -inf analogously)
// returns remaining costs k after merging
// if k<0 no solution for the graph and input k exists
int AbstractSolver::dataRed_weight_k(int k){
//    printDebug("Data reduction (weight > k=" + std::to_string(k) + "):");
    int k_before = k;

    redo:
    if(k < 0) {
//        printDebug("Fail: maximum cost exceeded");
        return -1;
    }
    for(int u : g->active_nodes){
        for(int v : g->active_nodes){
            if(u == v) continue;
            int weight_uv = g->get_weight(u,v);
            if(weight_uv > k){
                int cost = g->merge(u,v);
                if(cost == -1) return -1; // merging failed
                k -= cost;
                goto redo;
            }
            else if(weight_uv != DO_NOT_ADD && -weight_uv > k){
                g->set_non_edge(u,v);
            }
        }
    }
    return k;
}

int AbstractSolver::dataRed_heavy_non_edge(int k) {
    redo:
    if(k < 0) {
        return -1;
    }
    for(int u : g->active_nodes){
        // compute sum of {u,w} for all active neighbors w
        int weight_neighbours = 0;
        for(int w : g->active_nodes){
            if(u == w) continue;
            int weight = g->get_weight(u,w);

            // merge if {u,w} is DND
            if(weight == DO_NOT_DELETE) {
                int cost = g->merge(u,w);
                if(cost == -1) return -1;
                k -= cost;
                goto redo;
            }

            if(weight > 0){
                weight_neighbours += weight;
            }
        }

        // compute all heavy non-edges with u
        // for all v with {u,v} < 0, check if |{u,v}| >= sum
        for(int v : g->active_nodes){
            if(u == v) continue;
            int weight_uv = g->get_weight(u,v);
            if(weight_uv == DO_NOT_ADD) // {u,v} is already a heavy non edge
                continue;
            if(weight_uv < 0 && abs(weight_uv) >= weight_neighbours){
                g->set_non_edge(u,v);
            }
        }
    }
    return k;
}

int AbstractSolver::dataRed_heavy_edge_single_end(int k) {
    redo:
    if(k < 0) {
//        printDebug("Fail: maximum cost exceeded");
        return -1;
    }
    for(int u : g->active_nodes){
        // compute sum abs({u,w}) for all active neighbors w
        // save all DNA  edges of u for later; merge DND edges
        int weight_neighbours = 0;
        std::vector<int> dna = std::vector<int>();
        for(int w : g->active_nodes){
            if(u == w) continue;
            int weight = g->get_weight(u,w);

            // merge if {u,w} is DND
            if(weight == DO_NOT_DELETE) {
                int cost = g->merge(u,w);
                if(cost == -1) return -1;  // merging failed
                k -= cost;
                goto redo;
            }

            if(weight == DO_NOT_ADD)
                dna.push_back(w);
            else weight_neighbours += abs(weight);
        }

        // compute all heavy edges with u
        for(int v : g->active_nodes){
            if(u == v) continue;
            int weight_uv = g->get_weight(u,v);

            if(weight_uv < 0)
                continue;

            // merge if {u,v} is DND
            if(weight_uv == DO_NOT_DELETE){
                int cost = g->merge(u,v);
                if(cost == -1) return -1;
                k -= cost;
                goto redo;
            }

            // if {u,w} = dna, check if {w,v} agrees, otherwise add cost for changing {w,v}
            bool stop = false;
            for(int w: dna){
                if(g->get_weight(v,w) == DO_NOT_DELETE) {
                    stop = true;
                    break;
                }
                if(g->get_weight(v,w) > 0) {
                    weight_neighbours += g->get_weight(v,w);

                }
            }
            if(stop == true) continue;

            // merge if heavy edge condition holds
            if(weight_uv >= weight_neighbours - weight_uv){
                int cost = g->merge(u,v);
                if(cost == -1) return -1;
                k -= cost;
//                printDebug("Merged vertices for dataRed_heavy_edge_single_end: (" + std::to_string(u) + "," + std::to_string(v) + ")" );
                goto redo;
            }
        }
    }

    return k;
}


int AbstractSolver::dataRed_heavy_edge_both_ends(int k) {
    int cost = 0;
    redo:
    if(k < 0) {
//        printDebug("Fail: maximum cost exceeded");
        return -1;
    }
    for( int u: g->active_nodes){
        int max_weight = 0;
        int max_u = -1;
        int max_v = -1;
        for(int v : g->active_nodes){
            if(u == v) continue;
            int _weight = g->get_weight(u,v);
            if(_weight > max_weight){
                max_weight = _weight;
                max_u = u;
                max_v = v;
            }
        }
        if(max_u != -1){
            auto neighbours_of_u = g->closed_neighbourhood(max_u).first;
            auto neighbours_of_v = g->closed_neighbourhood(max_v).first;
            int sum_of_u = 0;
            int sum_of_v = 0;
            for(auto i: neighbours_of_u){
                if(i == max_u || i == max_v) continue;
                int weight = g->get_weight(i,max_u);
                if(weight != DO_NOT_DELETE && weight != DO_NOT_ADD)
                    sum_of_u += g->get_weight(i,max_u);
                else{
                    sum_of_u = DO_NOT_DELETE;
                    break;
                }
            }

            for(auto i : neighbours_of_v) {
                if (i == max_u || i == max_v) continue;
                int weight = g->get_weight(i, max_v);
                if(weight != DO_NOT_DELETE && weight != DO_NOT_ADD)
                    sum_of_v += g->get_weight(i, max_v);
                else{
                    sum_of_v = DO_NOT_DELETE;
                    break;
                }
            }

            int sum = sum_of_u + sum_of_v;
            if(sum_of_u > 0 && sum_of_v > 0 && sum < 0){
                sum = DO_NOT_DELETE;
            }
            if(sum_of_u < 0 && sum_of_v < 0 && sum > 0){
                sum = DO_NOT_ADD;
            }
            if(max_weight >= sum){
                int cost = g->merge(max_u, max_v);
                if(cost == -1) return -1;
                k -= cost;
                printDebug("Successful heavy_edge_both_ends merge costs: " + std::to_string(cost));
                goto redo;
            }
        }

    }

    return k;
}

int AbstractSolver::dataRed_remove_clique() {
    int N = g->merge_map.size();
    bool *visited = new bool[N];

    for(int i = 0; i < N; ++i){
        visited[i] = false;
    }
    std::vector<std::vector<int>> components;
    for(int i : g->active_nodes){
        if(visited[i] == false){
            components.push_back({});
            g->DFS(i, visited, components.back());
        }
    }

    for(auto &component : components){
        bool is_clique = true;
        for(int i = 0; i < component.size() && is_clique; ++i){
//            std::cout << component.at(i) + 1 << " ";
            for(int j = i +1 ; j < component.size(); ++j){
                // for {u,v} = 0 we consider u,v as not adjacent (we delete {u,v} later in unmerging)
                // thus, we have a clique only if {u,v} > 0
                if(g->get_weight(component.at(i), component.at(j)) <= 0){
                    is_clique = false;
                    break;
                }

            }
        }
        if(is_clique){
            g->remove_clique(component);
        }
    }

    delete[] visited;
    return 0;
}



//is doing the large Neighbourhood Rule for all vertices in the graph TODO
int AbstractSolver::dataRed_large_neighbourhood_I(int k) {

        int merge_costs = 0;
        rerun_after_merge:
        if(k < 0) {
            return -1;
        }
        for(int i : g->active_nodes){
            auto neighbourhoods = closed_neighbourhood(i);
            auto neighbours = neighbourhoods.first;
            auto not_neighbours = neighbourhoods.second;
            bool valid_neighborhood = true;
            //non of the edges is allowed to be 0 within the neighborhood
            for(auto n: neighbours){
                for(auto j : neighbours){
                    if(n == j) continue;
                    if(g->get_weight(n,j) == 0)
                        valid_neighborhood = false;
                }
            }

            //valid neighborhood(no edges have weight 0) so do the rest of the calculations
            int deficiency = this->deficiency(neighbours);
            int cut_weight = this->cut_weight(neighbours, not_neighbours);
            if(deficiency != DO_NOT_DELETE && cut_weight != DO_NOT_DELETE) {
                int sum_LN1 = 2 * deficiency + cut_weight;
                if(sum_LN1 < 0) // int overflow
                    sum_LN1 = DO_NOT_DELETE;
                if(valid_neighborhood && sum_LN1 < neighbours.size() && neighbours.size() >= 2){
                    int first = neighbours.front();
                    neighbours.pop_front();
                    int second = neighbours.front();
                    neighbours.pop_front();

                    int val = g->merge(first,second);
                    if(val == -1) return -1; // merging failed
                    merge_costs += val;

                    while(!neighbours.empty()){
                        int last_merged = g->active_nodes.back();
                        int next_from_neighbourhood = neighbours.front();
                        neighbours.pop_front();

                        val = g->merge(last_merged, next_from_neighbourhood);
                        if(val == -1) return -1; // merging failed
                        merge_costs += val;

                    }
                    printDebug("Successful large neighborhood 1, merge costs: " + std::to_string(merge_costs));
                    goto rerun_after_merge;
                }else if(neighbours.size() >= 2 && g->active_nodes.size() > 100){
                    int min_cut_weight = this->min_cut(neighbours);
                    int sum_LN2 = deficiency + cut_weight;
                    if(sum_LN2 <= min_cut_weight){
                        int first = neighbours.front();
                        neighbours.pop_front();
                        int second = neighbours.front();
                        neighbours.pop_front();

                        int val = g->merge(first,second);
                        if(val == -1) return -1; // merging failed
                        merge_costs += val;

                        while(!neighbours.empty()){
                            int last_merged = g->active_nodes.back();
                            int next_from_neighbourhood = neighbours.front();
                            neighbours.pop_front();

                            val = g->merge(last_merged, next_from_neighbourhood);
                            if(val == -1) return -1; // merging failed
                            merge_costs += val;
                        }
                        printDebug("Successful large neighborhood 2, merge costs: " + std::to_string(merge_costs));
                        goto rerun_after_merge;
                    }
                }
            }
        }
        k -= merge_costs;
        return k;
    }

// param: u is the index of the vertex of which the neighbours are collected
// return: - pair for neighbourhood(first item in pair) - all the vertices that are adjacent to u,
//         - not_neighbours(second item in pair) - all vertices that are not adjacent to u
//
    std::pair<std::list<int>, std::list<int>> AbstractSolver::closed_neighbourhood(int u) {
        std::list<int> neighbours;
        std::list<int> not_neighbours;
        for(int i : g->active_nodes){
            if(i == u) continue;
            if(g->get_weight(u,i) > 0){
                neighbours.push_back(i);
            }else if(g->get_weight(u,i) < 0){
                not_neighbours.push_back(i);
            }
        }
        neighbours.push_back(u);
        return std::make_pair(neighbours, not_neighbours);
    }

//calculates the costs to make the neighbourhood a clique
int AbstractSolver::deficiency(std::list<int> neighbours) {
    int costs = 0;
    while (!neighbours.empty()) {
        int i = neighbours.front();
        neighbours.pop_front();
        for (int j : neighbours) {
            if ( i == j) continue;
            if (g->get_weight(i, j) < 0) {
                if(g->get_weight(i,j) == DO_NOT_ADD){
                    return DO_NOT_DELETE; // abs(DO_NOT_ADD) is DO_NOT_ADD again but a high value should be returned
                }
                costs += abs(g->get_weight(i, j));
            }
        }
    }
    return costs;
}

//calculates the cost to cut of the neighbourhood(neighbourhood) from the rest of the graph(rest_graph)
int AbstractSolver::cut_weight(std::list<int>& neighbourhood, std::list<int>& rest_graph) {
    int cut_costs = 0;
    for(int i : neighbourhood){
        for(int j : rest_graph){
            if(i == j)continue;
            int weight = g->get_weight(i,j);
            if(weight > 0){
                if(weight == DO_NOT_DELETE){
                    return DO_NOT_DELETE;
                }
                cut_costs += weight;
            }
        }
    }
    return cut_costs;
}

int AbstractSolver::min_cut(const std::list<int>& neighbours) {
//    g->printGraph(std::cout);

    unsigned int all_possible_vertices = neighbours.size()*2;
    bool *active = new bool[all_possible_vertices];
    std::vector<int> active_nodes;

    //sub graph creation. create a graph that is only the neighborhood. Idea was not to get problem with the original
    // graph, because for the algorithm some merging needs to be done, but we dont need to reverse it
    std::vector<std::vector<std::pair<int,int>>> adj_list;
    int node_idx = 0;
    for(int i : neighbours){
        adj_list.emplace_back();
        int new_neighbour_idx = 0;
        for(int j : neighbours){
            if(i == j){
                ++new_neighbour_idx;
                continue;
            }
            int weight = g->get_weight(i,j);
//            std::cout << i << " " << j << std::endl;
            if(weight>0)
                adj_list.back().emplace_back(std::make_pair(new_neighbour_idx,weight));
            ++new_neighbour_idx;
        }
        active[node_idx] = true;
        active_nodes.push_back(node_idx);
        node_idx++;
    }



    int min_cut_weight = INT32_MAX;
    while(active_nodes.size() > 1) {
        //calculate the cut_weight of a min_cut_phase
        int cut_weight = min_cut_phase(adj_list, active, active_nodes, all_possible_vertices);
        if(cut_weight < min_cut_weight)
            min_cut_weight = cut_weight;

    }
    delete[] active;
    return min_cut_weight;
}

int AbstractSolver::min_cut_phase
        (std::vector<std::vector<std::pair<int,int>>>& G, bool *active, std::vector<int>& active_nodes, unsigned int num_possible_vertices) {
    //initialize
    std::vector<int> vertex_set;
    //this array is for faster lookups if a vertex is in the vertex set or not
    bool *is_in_vertex_set = new bool[num_possible_vertices];
    for(int i = 0; i < num_possible_vertices; ++i)
        is_in_vertex_set[i] = false;
    //picking the first element of active nodes as the first element in the vertex set
    int start_node = active_nodes[0];
    vertex_set.push_back(start_node);
    is_in_vertex_set[start_node]=true;
    int weight_of_last_added = 0;
    //the actual algorithm
    while (vertex_set.size() != active_nodes.size()){
        auto most_connected = get_most_tightly_connected(vertex_set, is_in_vertex_set, G, active);
        int most_connected_idx = most_connected.first;
        weight_of_last_added = most_connected.second;
        //adding the most connected vertex to the set
        vertex_set.push_back(most_connected_idx);
        is_in_vertex_set[most_connected_idx] = true;
    }

    //getting last two added vertices
    int last_added = vertex_set.back();
    vertex_set.pop_back();
    int second_last_added = vertex_set.back();
    vertex_set.pop_back();

    //merge the last two added nodes
    //add a new node at the back of the G and the active nodes
    active_nodes.push_back((int)G.size());
    G.emplace_back();

    for(auto it = active_nodes.begin(); it != active_nodes.end();++it){
        //delete the two vertices that have to be merged
        if(*it == last_added || *it == second_last_added) {
            active_nodes.erase(it);
            --it;
            continue;
        }else{
            //got over the all the neighbors of an active node
            int weight_to_merged_node = 0;
            for(auto& neigh: G[*it]){
                int neigh_idx = neigh.first;
                int neigh_weight = neigh.second;
                //set the idx of the old not existent nodes to the new node(the merged node)
                //i only change the idx, and do not add the two weight together because it is not necessary, because
                //the weights are only important to get the most_tightly_connected node, and the way how i calculate this
                //would just add all the weight with the same idx together
                if(neigh_idx == last_added || neigh_idx == second_last_added){
                    neigh.first = G.size()-1;
                    weight_to_merged_node += neigh_weight;
                }
            }
            //add an edge to the merged node to the current node
            if(weight_to_merged_node != 0){
                G[G.size()-1].push_back(std::make_pair(*it, weight_to_merged_node));
            }
        }
    }

    //delete merged nodes from active nodes
    active[last_added] = false;
    active[second_last_added] = false;
    active[G.size()-1] = true;


    //calculate cut weight...maybe its the same as most_connected_weight
    int cut_weight = 0;
    for(auto neigh : G[last_added]){
        cut_weight += neigh.second;
    }
    if(cut_weight != weight_of_last_added){
        throwError("cut weight are not equal!! cut weight :" + std::to_string(cut_weight) + " weight of last added " + std::to_string(weight_of_last_added));
    }


    delete[] is_in_vertex_set;
    return cut_weight;
}

std::pair<int,int> AbstractSolver::get_most_tightly_connected
        (const std::vector<int>& vertex_set,const bool *is_in_vertex_set, std::vector<std::vector<std::pair<int,int>>>& G, const bool *active_nodes) {

    //for calculating the sum from any vertex not in vertex set to the vertex set
    auto *sum_of_edges_to_subset = new int[G.size()];
    for(int i = 0; i < G.size(); ++i){
        sum_of_edges_to_subset[i] = 0;
    }

    int max_weight = 0;
    int max_weight_idx = -1;
    //go through all the nodes in the vertex set and then go through all the neighbours and add the weight,
    // find the max weighted connection to the vertex set
    for(int v : vertex_set){
        for(auto& neigh : G[v]){
            int neighbor_idx = neigh.first;
            int neighbor_weight = neigh.second;
            if(!is_in_vertex_set[neighbor_idx] && active_nodes[neighbor_idx]){
                if(neighbor_weight == DO_NOT_DELETE){
                    sum_of_edges_to_subset[neighbor_idx] = DO_NOT_DELETE;
                }else if(sum_of_edges_to_subset[neighbor_idx] == DO_NOT_DELETE){
                    sum_of_edges_to_subset[neighbor_idx] = DO_NOT_DELETE;
                }else{
                    sum_of_edges_to_subset[neighbor_idx] += neighbor_weight;
                }
                if(sum_of_edges_to_subset[neighbor_idx] > max_weight){
                    max_weight = sum_of_edges_to_subset[neighbor_idx];
                    max_weight_idx = neighbor_idx;
                }
            }
        }
    }
    delete[] sum_of_edges_to_subset;
    return std::make_pair(max_weight_idx, max_weight);
}

