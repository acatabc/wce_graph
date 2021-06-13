#include <ilcplex/ilocplex.h>
#include "../lib/Solver.h"
#include "../include/utils.h"

int main(){
  Solver *s = new Solver();
  int initial_number_of_nodes = s->g->active_nodes.size();
  s->data_reduction_before_branching();

  std::vector<int> original_graph;
  unsigned int active_nodes_size = s->g->active_nodes.size();
  for(int i = 0; i < active_nodes_size; ++i){
    for(int j = i +1; j < active_nodes_size;++j){
        int node_u = s->g->active_nodes.at(i);
        int node_v = s->g->active_nodes.at(j);
        if(s->g->get_weight(node_u, node_v) > 0){
            original_graph.push_back(1);
        }else{
            original_graph.push_back(0);
        }
    }
  }

  //Model creation
	IloEnv env;
	IloModel model(env);
    IloNumVarArray var(env);
    IloRangeArray c(env);
    IloNumExpr expr(env);

    //adding variables e_uv to the model
    unsigned int num_edges = 0;
    std::vector<std::pair<int,int>> lookup_table;
    for(int u = 0; u < active_nodes_size; ++u){
      for(int v = u+1; v < active_nodes_size; ++v){
        int node_u = s->g->active_nodes.at(u);
        int node_v = s->g->active_nodes.at(v);
        //std::cout << node_u << " " << node_v << std::endl;
        var.add(IloNumVar(env, 0,1,ILOINT));
       // std::cout <<"size "<< var.getSize() << std::endl;
        lookup_table.push_back(std::make_pair(node_u,node_v));
       
        ++num_edges;

        //expression building: function to minimize
        int weight = s->g->get_weight(node_u, node_v);
        if(weight > 0){
            expr += (-var[num_edges-1])*weight;
        }else{
            if(weight == DO_NOT_ADD)
              expr += var[num_edges-1]*DO_NOT_DELETE;
            else
                expr += var[num_edges-1]*abs(weight);
        }
  //  std::cout << expr << std::endl;
      }
    }
   // std::cout << expr << std::endl;
    //adding constraints

    for(int u = 0; u < num_edges ; ++u){
      for(int v = u+1; v < num_edges; ++v){
        for(int w = v+1; w < num_edges; ++w){
          auto x = lookup_table.at(u);
          auto y = lookup_table.at(v);
          auto z = lookup_table.at(w);
          if(x.first == y.first && x.second == z.first && z.second == y.second){
            c.add(var[u]+var[v]-var[w] <= 1);
            c.add(var[u]-var[v]+var[w] <= 1);
            c.add(-var[u]+var[v]+var[w] <= 1);
          }
        
        }
      }
    }



   //std::cout <<"CONSTRAINTS : "<< std::endl << c << std::endl;

    model.add(IloMinimize(env, expr));
    model.add(c);
	IloCplex cplex(model);
    cplex.setOut(env.getNullStream());
	cplex.solve();
        /*std::cout <<"active nodes" << std::endl;
        for(int i : s->g->active_nodes){
            std::cout << i << std::endl;
        }
        std::cout <<"end active nodes" << std::endl;
*/

    for(int i = 0; i  < num_edges ; ++i){
        //auto b = cplex.getValue(var[i]);
        auto val = cplex.getIntValue(var[i]);
        auto orig = original_graph.at(i);
  //      std::cout <<i<<" ("<< lookup_table.at(i).first+1 <<","<<lookup_table.at(i).second+1 << ") " << orig << " " << val << std::endl;
        if(val^orig){
            auto p = lookup_table.at(i);
            //std::cout << p.first+1 << " " << p.second+1 << std::endl;
           if(val > 0){
                if(p.first < initial_number_of_nodes && p.second < initial_number_of_nodes)
                    std::cout << p.first+1 << " " << p.second+1 << std::endl;
                else
                    s->g->add_edge(p.first,p.second);
                
           }
           else{
                if(p.first < initial_number_of_nodes && p.second < initial_number_of_nodes)
                    std::cout << p.first+1 << " " << p.second+1 << std::endl;
                else
                    s->g->delete_edge(p.first,p.second);
           }
        }
    }
    s->clear_stack_and_output();

	env.end();
	return 0;
}
