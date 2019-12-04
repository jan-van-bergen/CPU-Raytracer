# Advanced Graphics - Assignment 1

## Basic Features
- [x] Generic and extendible architecture for a Raytacer,
- [x] Free camera with configurable position, orientation, fov and aspect ratio.
- [x] Camera can be moved using WASD keys and rotated using arrow keys.
- [x] Supports planes and spheres.
- [x] Basic but extendible material class.
- [x] Basic Scene.
- [x] Whitted-style ray tracing, with support for shadows and reflections.

## Bonus Features
- [x] Support for Triangle Meshes using `.obj` files.
- [x] Texture mapping for all Primitives (Spheres, Planes, and Triangle Meshes).
- [x] Support for Point Lights, Spot Lights, and Directional Lights.
- [x] Full support for Dielectrics; Snell, Fresnel, and Beer.
- [x] Efficient tile-based multithreading using the Windows API.
- [x] Optimized Renderer: Triangle/Ray intersection makes use of SIMD.
- [x] Skydome using HDR light probes (https://www.pauldebevec.com/Probes/).

## Project
Two Scenes are available. You can swtich between these by changing the `#define CURRENT_SCENE` to either `SCENE_TEST` or `SCENE_WHITTED` in `Scene.h`. The number of bounces is also configurable here, the default is 3.

The project uses a somewhat data-oriented design. I took data layout into consideration, and made sure there are no virtual function calls. Allmost all parts of the program use SIMD. I wrote abtractions over the intrinsics to make use of features such as operator overloading. This also allows for switching between 4 lane SIMD (SSE) and 8 lane SIMD (AVX), using the `#define SIMD_LANE_SIZE` found in `SIMD.h`.

## Code Attribution
- Ray-Triangle intersection was based on code from [Wikipedia](https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm). 
- Multithreading code was based on various code samples from the INFOMOV slides on multithreading.
