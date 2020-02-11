#pragma once
// Scene settings
#define SCENE_SPONZA  0 
#define SCENE_DYNAMIC 1

#define SCENE SCENE_SPONZA

// Render settings
#define SCREEN_WIDTH  900
#define SCREEN_HEIGHT 600

#define NUMBER_OF_BOUNCES 3

#define USE_MULTITHREADING true // When enabled will use the maximum amount of threads available

#define SIMD_LANE_SIZE 8

// BVH settings
#define BVH_VISUALIZE_HEATMAP false // Toggle to visualize number of traversal steps through BVH

#define BVH_TRAVERSE_TREE_NAIVE   0 // Traverses the BVH in a naive way, always checking the left Node before the right Node
#define BVH_TRAVERSE_TREE_ORDERED 1 // Traverses the BVH based on the split axis and the direction of the Ray

#define BVH_TRAVERSAL_STRATEGY BVH_TRAVERSE_TREE_ORDERED

#define MESH_USE_BVH  0
#define MESH_USE_SBVH 1

#define MESH_ACCELERATOR MESH_USE_SBVH // Bottom Level (object space) acceleration structure

// Texture settings
#define TEXTURE_SAMPLE_MODE_NEAREST  0
#define TEXTURE_SAMPLE_MODE_BILINEAR 1
#define TEXTURE_SAMPLE_MODE_MIPMAP   2

#define TEXTURE_SAMPLE_MODE TEXTURE_SAMPLE_MODE_MIPMAP

// Ray Differentials are only used to determine the correct mipmap level,
// so we don't need to compute them if mipmapping is not enabled
#define RAY_DIFFERENTIALS_ENABLED (TEXTURE_SAMPLE_MODE == TEXTURE_SAMPLE_MODE_MIPMAP)

// Mipmap settings
#define MIPMAP_FILTER_TRILINEAR 0
#define MIPMAP_FILTER_EWA       1

#define MIPMAP_FILTER MIPMAP_FILTER_EWA

#define MAX_ANISOTROPY 8.0f
