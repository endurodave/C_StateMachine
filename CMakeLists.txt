# Example CMake command line to create project build files:
#
# *** Windows ***
# cmake -G "Visual Studio 17 2022" -A Win32 -B ../C_StateMachineBuild -S .
#
# *** Linux ***
# cmake -G "Unix Makefiles" -B ../C_StateMachineBuild -S .

# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language (C or C++)
project(C_StateMachine VERSION 1.0 LANGUAGES C CXX)

# Collect all source files in the current directory
file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/*.c"
)

# Add an executable target
add_executable(C_StateMachineApp ${SOURCES})



