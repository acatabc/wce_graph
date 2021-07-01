#include <ilcplex/ilocplex.h>
#include "../lib/Solver.h"
#include "../include/utils.h"



int main(){
    WCE_Graph *g = parse_and_build_graph();
  Solver *s = new Solver(g);
  s->data_reduction_before_branching();
  unsigned int active_nodes_size = s->g->active_nodes.size();



  //Model creation
	IloEnv env;
	IloModel model(env);
    IloNumVarArray var(env);
    IloRangeArray c(env);
    IloExpr expr(env);

    //adding variables e_uv to the model
    unsigned int num_edges = 0;
    std::vector<std::pair<int,int>> edge_table;
    for(int u = 0; u < active_nodes_size; ++u){
      for(int v = u+1; v < active_nodes_size; ++v){
        int node_u = s->g->active_nodes.at(u);
        int node_v = s->g->active_nodes.at(v);
        //std::cout << node_u << " " << node_v << std::endl;
        var.add(IloNumVar(env, 0.0,1.0,ILOFLOAT));
       // std::cout <<"size "<< var.getSize() << std::endl;
//        edge_table.push_back(std::make_pair(node_u,node_v));
       
        ++num_edges;

        //expression building: function to minimize
        int weight = s->g->get_weight(node_u, node_v);
        if(weight > 0){
            expr += (1.0-var[num_edges-1])*weight;
        }else{
            if(weight == DO_NOT_ADD)
              expr += var[num_edges-1]*DO_NOT_DELETE;
            else
                expr += var[num_edges-1]*abs(weight);
        }
      }
    }

    //adding constraints

    for(int u = 0; u < active_nodes_size; ++u){
      for(int v = u+1; v < active_nodes_size; ++v){
        for(int w = v+1; w < active_nodes_size; ++w){
            int uv = u*active_nodes_size - (u*(u-1))/2 + (v-u)-1 ;
            int vw = v*active_nodes_size - (v*(v-1))/2 + (w-v)-1 ;
            int uw = u*active_nodes_size - (u*(u-1))/2 + (w-u)-1 ;
            c.add(1.0*var[uv]+1.0*var[vw]-1.0*var[uw] <= 1.0);
            c.add(1.0*var[uv]-1.0*var[vw]+1.0*var[uw] <= 1.0);
           c.add(-1.0*var[uv]+1.0*var[vw]+1.0*var[uw] <= 1.0);
            
        }
      }
    }



   //std::cout <<"CONSTRAINTS : "<< std::endl << c << std::endl;

    model.add(IloMinimize(env, expr));
    model.add(c);
	IloCplex cplex(model);
//    cplex.setParam(IloCplex::RootAlg, IloCplex::Dual);
    cplex.setOut(env.getNullStream());
	cplex.solve();

/*+++++++++++++++++ output start +++++++++++++++++++++++
    for(int i = 0; i  < num_edges ; ++i){
        //auto b = cplex.getValue(var[i]);
        auto val = cplex.getIntValue(var[i]);
        auto orig = original_graph.at(i);
  //      std::cout <<i<<" ("<< edge_table.at(i).first+1 <<","<<edge_table.at(i).second+1 << ") " << orig << " " << val << std::endl;
        if(val^orig){
            auto p = edge_table.at(i);
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
    //   ++++++++++++++++++ output end +++++++++++++++++++++++++*/
//    for(int i = 0; i < num_edges; ++i){
//      double val = cplex.getValue(var[i]);
 //       std::cout << i << " " << val<< std::endl;
   // }

//    double lower_bound = cplex.getObjValue();
//    std::cout << "# lower bound" << lower_bound<< std::endl;

   env.end();
	return 0;
}
