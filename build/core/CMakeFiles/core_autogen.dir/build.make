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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lev/Zyrlo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lev/Zyrlo/build

# Utility rule file for core_autogen.

# Include the progress variables for this target.
include core/CMakeFiles/core_autogen.dir/progress.make

core/CMakeFiles/core_autogen:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/lev/Zyrlo/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Automatic MOC and UIC for target core"
	cd /home/lev/Zyrlo/build/core && /usr/local/bin/cmake -E cmake_autogen /home/lev/Zyrlo/build/core/CMakeFiles/core_autogen.dir/AutogenInfo.json Debug

core_autogen: core/CMakeFiles/core_autogen
core_autogen: core/CMakeFiles/core_autogen.dir/build.make

.PHONY : core_autogen

# Rule to build all files generated by this target.
core/CMakeFiles/core_autogen.dir/build: core_autogen

.PHONY : core/CMakeFiles/core_autogen.dir/build

core/CMakeFiles/core_autogen.dir/clean:
	cd /home/lev/Zyrlo/build/core && $(CMAKE_COMMAND) -P CMakeFiles/core_autogen.dir/cmake_clean.cmake
.PHONY : core/CMakeFiles/core_autogen.dir/clean

core/CMakeFiles/core_autogen.dir/depend:
	cd /home/lev/Zyrlo/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lev/Zyrlo /home/lev/Zyrlo/core /home/lev/Zyrlo/build /home/lev/Zyrlo/build/core /home/lev/Zyrlo/build/core/CMakeFiles/core_autogen.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : core/CMakeFiles/core_autogen.dir/depend

