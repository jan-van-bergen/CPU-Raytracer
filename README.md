# Advanced Graphics - Assignment 2



## Bonus Features
- [x] SBVH was implemented.
- [x] (Regular) BVH Construction of a scene with >= 100000 triangles is done in under one second.
- [x] BVH and SBVH traversal is optimized by expanding child nodes in order, based on the split axis and the Ray's direction. The split axis is stored in the two highest bits of the ``count`` fields of ``BVHNode`` and ``SBVHNode`` to ensure the fact that those structs are still 32 bytes.
- [x] The BVH and SBVH are traversed using SIMD, allowing for 4 simulateous Rays. This results in a speedup of about 6x over scalar code!. You can switch between scalar flow, and vector flow by changing the ``SIMD_LANE_SIZE``define in SIMD.h.

## Attribution
- Ray-AABB intersection was based on code from https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
