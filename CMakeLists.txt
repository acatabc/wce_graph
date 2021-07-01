cmake_minimum_required(VERSION 3.10)
project(algeng_wce_solver)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

add_executable(
        algeng_wce_solver
        main.cpp
        include/utils.cpp include/utils.h
        lib/WCE_Graph.cpp lib/WCE_Graph.h
        lib/AbstractSolver.h lib/AbstractSolver.cpp
        lib/HeuristicSolver.h lib/HeuristicSolver.cpp
        lib/Solver.h lib/Solver.cpp
        lib/abstractSolver_dataReduction.cpp lib/solver_P3Search.cpp
)
