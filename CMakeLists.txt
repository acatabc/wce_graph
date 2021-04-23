cmake_minimum_required(VERSION 3.19)
project(algeng_wce_solver)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
add_executable(algeng_wce_solver main.cpp lib/WCE_Graph.cpp lib/WCE_Graph.h lib/Solver.cpp lib/Solver.h lib/parser.cpp include/utils.h)
#add_executable(edit_weight_graph main_2.cpp include/cluster_graph.h)
