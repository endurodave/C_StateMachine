# allocator

A copy of [C_Allocator](https://github.com/endurodave/C_Allocator) 2.1.0 (its `src/` folder), used by this repository's CMake build.

The PlatformIO package does not include these files: it depends on `endurodave/C_Allocator` instead, so a project can use C_StateMachine and C_Allocator together without duplicate symbols.
