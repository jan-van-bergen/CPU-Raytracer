# Advanced Graphics - Assignment 2

## Features

### SBVH
SBVH was implemented, including reference unsplitting. BVH and SBVH use the same struct ``BVH``, you can switch between them by  calling either ``BVHBuilder::build_bvh(...)`` or ``BVHBuilder::build_sbvh(...)``. Note that the SBVH only supports Triangle primitives.

The SBVH is used when the ``MESH_ACCELERATOR`` define in Mesh.h is set to ``MESH_USE_SBVH``.

### Fast BVH Construction
Regular BVH Construction of a scene with > 100000 triangles is done in under one second. The Sponza scene (262205 triangles) takes less than 700 ms on my machine.

The regular BVH is used when the ``MESH_ACCELERATOR`` define in Mesh.h is set to ``MESH_USE_BVH``.

SBVH construction of Sponza takes about 12 seconds on my machine.

### Fast BVH Traversal
BVH and SBVH traversal is optimized by expanding child nodes in order, based on the split axis and the Ray's direction. The split axis is stored in the two highest bits of the ``count`` fields of ``BVHNode`` and ``SBVHNode`` to ensure the fact that those structs are still 32 bytes.

Both the BVH and SBVH are traversed using SIMD, allowing for 4 simulateous Rays. This results in a speedup of about 6x over scalar code!. You can switch between scalar flow, and vector flow by changing the ``SIMD_LANE_SIZE``define in SIMD.h.

## Attribution
- Ray-AABB intersection was based on code from https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
