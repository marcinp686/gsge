# GSGE
Giraffe Studio Game Engine is a place to learn some Vulkan 1.3 API.
Well, it's not exactly a game engine. I aim to make it a benchmarking app for GPUs using Vulkan API.<br><br>
When I started learning Vulkan, I faced one major issue: Which way of making things is correct and fastest?<br> Should I use the same Queue Family Index for the graphics and presentation queues? I read that using a dedicated transfer queue makes memory transfers faster, but does it always? And faster by how much? GPU vendors tend to give general guidelines on how to do things, but does it work for each GPU from the same vendor? Does an RX 6700XT benefit from a certain solution by the same amount as an RX 7900XT? Is dynamic rendering faster than "traditional"?<br> this project aims to give measurements to different rendering methods using Vulkan so we can conclude off of it.<br><br>
Additionally, I noticed that Vulkan implementation is not always correct across vendors. The same code, the same specification, a different vendor, and slightly different behavior. It would be nice to pinpoint these issues (one such issue is resource ownership across different queues in Vulkan 1.3 / synchronization2).

## Current state
Making architecture decisions, for now it's a bit messy.

## Dependencies
* [spdlog](https://github.com/gabime/spdlog) - logging
* [glfw](https://github.com/glfw/glfw) - window creation, input handling
* [DirectXMath](https://github.com/microsoft/DirectXMath) - all inline SIMD C++ linear algebra library
* [glm](https://github.com/g-truc/glm) - header only mathematics library
* [assimp](https://github.com/assimp/assimp) - model loading
* [fastgltf](https://github.com/spnda/fastgltf) - model loading, not used yet
* [entt](https://github.com/skypjack/entt) - Entity Component System
* [vulkan-memory-allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Memory allocation, not used yet
* [tracy](https://github.com/wolfpld/tracy) - profiling, with nice GUI

## Installation
GSGE uses vcpkg to handle dependencies, except for _easy_profiler_.
If using VS's vcpkg extension, it should download, build, and install necessary dependencies automatically.
If not, use `vcpkg install --triplet=x64-windows-static` in the solution folder.

## Configuration
There are no specific options to configure. However, for maximum performance, please set the architecture for code generation in project settings [/arch:AVX...](https://learn.microsoft.com/en-us/cpp/build/reference/arch-x64).

## Build
GSGE is available as a VS 2022 project, with Debug, Release, and Profile configurations available.<br>
Make sure to install _easy_profiler_ when using the "Profile" configuration.<br>
In the project settings, choose the right architecture for code generation for your CPU (AVX, AVX2, AVX512, and so on).

## Command-line parameters
|Parameter|Value type|Description|Default value|Example|
|---|---|---|---|---|
|--monitor|Integer>=0|Index of monitor to run app on|0|--monitor=0|
|--fullscreen|none|Run app fullscreen on selected monitor|not selected|--fullscreen|
|--windowed|none|Run app in window on selected monitor|selected|--windowed|
|--width|Integer>=1|Window width in windowed mode / Screen width in fullscreen mode|800|--width=800|
|--height|Integer>=1|Window height in windowed mode / Screen height in fullscreen mode|600|--height=600|

## Navigation/keys in the app
|Key|Description|
|---|---|
|W,S,A,D|Move forward, backward, strafe left, strafe right|
|Q|Move up|
|E|Move down|
|M|Toggle Multisampling at runtime|
|P|Pause/Run engine|
|Esc|Exit program|

## Contact me
e-mail: marcinp689@outlook.com<br>
discord: [highgradegiraffe](https://discord.com/users/highgradegiraffe)
