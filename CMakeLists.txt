# Example CMake command line to create project build files:
#
# *** Windows ***
# cmake -G "Visual Studio 17 2022" -A Win32 -B Build -S .
#
# *** Linux ***
# cmake -G "Unix Makefiles" -B Build -S .

# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language (C or C++)
project(C_StateMachine VERSION 1.0 LANGUAGES C CXX)

# Collect all source files in the current directory
file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/*.c"
    "${CMAKE_SOURCE_DIR}/*.h"
    # Copy of C_Allocator (https://github.com/endurodave/C_Allocator) for this
    # build. The PlatformIO package depends on endurodave/C_Allocator instead.
    "${CMAKE_SOURCE_DIR}/allocator/*.cpp"
    "${CMAKE_SOURCE_DIR}/allocator/*.c"
    "${CMAKE_SOURCE_DIR}/allocator/*.h"
)

# Add an executable target
add_executable(C_StateMachineApp ${SOURCES})
target_include_directories(C_StateMachineApp PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/allocator)



