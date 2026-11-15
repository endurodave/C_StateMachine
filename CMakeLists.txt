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

file(GLOB SOURCES
    # The library (the Arduino library layout: src/)
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    # The desktop demo: Motor and CentrifugeTest state machines
    "${CMAKE_SOURCE_DIR}/examples/desktop/*.c"
    "${CMAKE_SOURCE_DIR}/examples/desktop/*.h"
    # Copy of C_Allocator (https://github.com/endurodave/C_Allocator) for this
    # build. The PlatformIO and Arduino packages depend on C_Allocator instead.
    "${CMAKE_SOURCE_DIR}/allocator/*.cpp"
    "${CMAKE_SOURCE_DIR}/allocator/*.c"
    "${CMAKE_SOURCE_DIR}/allocator/*.h"
)

# Add an executable target
add_executable(C_StateMachineApp ${SOURCES})
target_include_directories(C_StateMachineApp PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/examples/desktop
    ${CMAKE_SOURCE_DIR}/allocator)
