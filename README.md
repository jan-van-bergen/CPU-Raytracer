# Advanced Graphics - Assignment 2

Student: Jan van Bergen - 5656877

## Features

### Surface Area Heuristic
The regular BVH uses the SAH to construct a good quality BVH. The SAH is evaluated on a per object basis, meaning no binning is used here.

### SBVH
SBVH was implemented, including reference unsplitting. BVH and SBVH use the same class ``BVH``, you can switch between them by  changing the ``MESH_ACCELERATOR`` define in MeshData.cpp to either ``MESH_USE_BVH`` or ``MESH_USE_SBVH``. 
I used the same settings as described in the paper (alpha=10^-5 and 256 bins).

The SBVH is used when the ``MESH_ACCELERATOR`` define in MeshData.cpp is set to ``MESH_USE_SBVH``.

### Two Levels of BVH
There is support for BVH at the Triangle level as well as at the Mesh level (Top Level BVH). The Top Level BVH is recomputed every frame, allowing for dynamic scenes with rigid motion. 
Triangle level BVH's are computed once per Mesh, meaning multiple instances of the same Mesh can reuse the same BVH using different world transforms.

The SBVH only supports Triangle primitives, so it cannot be used as a Top Level BVH.

### Fast BVH Construction
Regular BVH Construction of a scene with > 100000 triangles is done in under one second, even without binning. The Sponza scene (262205 triangles) takes less than 700 ms on my machine.

The easiest way of verifying this is setting the ``SCENE`` define in Scene.cpp to ``SCENE_SPONZA`` and the ``MESH_ACCELERATOR`` define in MeshData.cpp to ``MESH_USE_BVH``. The time it takes to construct a Triangle BVH is always reported for every Mesh.

SBVH construction of Sponza takes about 12 seconds on my machine.

### Fast BVH Traversal
BVH and SBVH traversal is optimized by expanding child nodes in order, based on the split axis and the Ray's direction. The split axis is stored in the two highest bits of the ``count`` fields of ``BVHNode`` and ``SBVHNode`` to ensure the fact that those structs are still 32 bytes.

You can switch between Naive (left first) and Ordered (nearest first) traversal strategies using the ``BVH_TRAVERSAL_STRATEGY`` define in BVH.h.

Both the BVH and SBVH are traversed using SIMD, allowing for 4 simulateous Rays. This results in a speedup of about 6x over scalar code!. You can switch between scalar flow, and vector flow by changing the ``SIMD_LANE_SIZE``define in SIMD.h.

## General

Two Scene configurations are provided; one only containing a single Sponza mesh to showcase the SBVH, and one containing various  moving meshes to showcase the Top Level BVH and Mesh Instancing.
You can switch between these by setting the ``SCENE`` define at the top of the Scene.cpp file.

The Camera can be controlled using WASD for movement and the arrow keys for orientation. Shift and Spacebar can be used to move vertically.

## Attribution
- Ray-AABB intersection was based on code from https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525

The following two codebases were used as reference when implementing SBVH. I did not copy their code, I only used it as a reference when I got stuck on various details of the algorithm. (For example, how to avoid floating point precision issues and how to handle some edge cases involving split plane straddlers)
- https://github.com/trungtle/TLVulkanRenderer
- https://github.com/shocker-0x15/SLR
