#pragma once
// Scene settings
#define SCENE_SPONZA  0
#define SCENE_DYNAMIC 1

#define SCENE SCENE_SPONZA

// Render settings
#define SCREEN_WIDTH  900
#define SCREEN_HEIGHT 600

#define NUMBER_OF_BOUNCES 3 // Number of bounces AFTER primary Rays, meaning 0 has only primary Rays

#define USE_MULTITHREADING true // When enabled will use the maximum amount of threads available

#define SIMD_LANE_SIZE 8 // 1 means scalar flow, 4 means SSE, 8 means AVX

#define MAX_MATERIALS 256 // Size of the global Material buffer

#define ENABLE_FXAA true // Fast Approximative Anti-Aliasing

// BVH settings
#define BVH_VISUALIZE_HEATMAP false // Toggle to visualize number of traversal steps through BVH

#define BVH_TRAVERSAL_STACK_SIZE 64

#define BVH_TRAVERSE_TREE_NAIVE   0 // Traverses the BVH in a naive way, always checking the left Node before the right Node
#define BVH_TRAVERSE_TREE_ORDERED 1 // Traverses the BVH based on the split axis and the direction of the Ray

#define BVH_TRAVERSAL_STRATEGY BVH_TRAVERSE_TREE_ORDERED

#define MESH_USE_BVH  0 // Regular SAH based BVH construction
#define MESH_USE_SBVH 1 // Spatial BVH. Able to split Triangles (see https://www.nvidia.in/docs/IO/77714/sbvh.pdf)

#define MESH_ACCELERATOR MESH_USE_SBVH // Bottom Level (object space) acceleration structure

// Texture settings
#define TEXTURE_SAMPLE_MODE_NEAREST  0 // No filtering
#define TEXTURE_SAMPLE_MODE_BILINEAR 1 // Bilinear filtering
#define TEXTURE_SAMPLE_MODE_MIPMAP   2 // Generates mipmaps for all Textures

#define TEXTURE_SAMPLE_MODE TEXTURE_SAMPLE_MODE_MIPMAP

// Ray Differentials are only used to determine the correct mipmap level,
// so we don't need to compute them if mipmapping is not enabled
#define RAY_DIFFERENTIALS_ENABLED (TEXTURE_SAMPLE_MODE == TEXTURE_SAMPLE_MODE_MIPMAP)

// Mipmap settings
#define MIPMAP_FILTER_TRILINEAR   0 // BAD  QUALITY - BEST PERFORMANCE - Lerps between two closest mipmaps (equivalent of GL_NEAREST_MIPMAP_LINEAR in OpenGL)
#define MIPMAP_FILTER_ANISOTROPIC 1 // GOOD QUALITY - GOOD PERFROMANCE - OpenGL style anisotropic filtering (see https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_filter_anisotropic.txt)
#define MIPMAP_FILTER_EWA         2 // BEST QUALITY - BAD  PERFORMANCE - Elliptical Weighted Average filter, also anisotropic. Great results but expensive to compute

#define MIPMAP_FILTER MIPMAP_FILTER_ANISOTROPIC

#define MAX_ANISOTROPY 8.0f // Used by both MIPMAP_FILTER_ANISOTROPIC and MIPMAP_FILTER_EWA
