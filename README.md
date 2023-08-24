# gsge
Giraffe Studio Game Engine, a place to learn some Vulkan 1.3 API
## Installation
GSGE uses vcpkg to handle dependencies, except for [EasyProfiler](https://github.com/yse/easy_profiler).
If using VS vcpkg extension it should download, build and install necessary dependencies automatically.
If not use `vcpkg install --triplet=x64-windows-static` in solution folder.

## Configuration
There are no specific options to configure. However, please set the architecture for code generation in project settings [/arch:AVX...](https://learn.microsoft.com/en-us/cpp/build/reference/arch-x64) for maximum performance.

## Build
GSGE is available as VS 2022 project, with Debug, Release and Profile configurations available. Make sure to install EasyProfiler when using "Profile" configuration. 

