# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/acat/Dokumente/clion-2020.3.2/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/acat/Dokumente/clion-2020.3.2/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/acat/CLionProjects/algeng_wce_solver

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/acat/CLionProjects/algeng_wce_solver/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/algeng_wce_solver.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/algeng_wce_solver.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/algeng_wce_solver.dir/flags.make

CMakeFiles/algeng_wce_solver.dir/main.cpp.o: CMakeFiles/algeng_wce_solver.dir/flags.make
CMakeFiles/algeng_wce_solver.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/acat/CLionProjects/algeng_wce_solver/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/algeng_wce_solver.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/algeng_wce_solver.dir/main.cpp.o -c /home/acat/CLionProjects/algeng_wce_solver/main.cpp

CMakeFiles/algeng_wce_solver.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/algeng_wce_solver.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/acat/CLionProjects/algeng_wce_solver/main.cpp > CMakeFiles/algeng_wce_solver.dir/main.cpp.i

CMakeFiles/algeng_wce_solver.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/algeng_wce_solver.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/acat/CLionProjects/algeng_wce_solver/main.cpp -o CMakeFiles/algeng_wce_solver.dir/main.cpp.s

# Object files for target algeng_wce_solver
algeng_wce_solver_OBJECTS = \
"CMakeFiles/algeng_wce_solver.dir/main.cpp.o"

# External object files for target algeng_wce_solver
algeng_wce_solver_EXTERNAL_OBJECTS =

algeng_wce_solver: CMakeFiles/algeng_wce_solver.dir/main.cpp.o
algeng_wce_solver: CMakeFiles/algeng_wce_solver.dir/build.make
algeng_wce_solver: CMakeFiles/algeng_wce_solver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/acat/CLionProjects/algeng_wce_solver/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable algeng_wce_solver"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/algeng_wce_solver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/algeng_wce_solver.dir/build: algeng_wce_solver

.PHONY : CMakeFiles/algeng_wce_solver.dir/build

CMakeFiles/algeng_wce_solver.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/algeng_wce_solver.dir/cmake_clean.cmake
.PHONY : CMakeFiles/algeng_wce_solver.dir/clean

CMakeFiles/algeng_wce_solver.dir/depend:
	cd /home/acat/CLionProjects/algeng_wce_solver/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/acat/CLionProjects/algeng_wce_solver /home/acat/CLionProjects/algeng_wce_solver /home/acat/CLionProjects/algeng_wce_solver/cmake-build-debug /home/acat/CLionProjects/algeng_wce_solver/cmake-build-debug /home/acat/CLionProjects/algeng_wce_solver/cmake-build-debug/CMakeFiles/algeng_wce_solver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/algeng_wce_solver.dir/depend

