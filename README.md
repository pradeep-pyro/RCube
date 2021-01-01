<img src="misc/rcube_logo.png" alt="RCube" width="250"/>


**RCube** (for <img src="https://render.githubusercontent.com/render/math?math=\mathbb{R}^3">) is an easy to use OpenGL 4.5 engine for 3D graphics/visualization. The library is a work-in-progress but usable. Some features of RCube include:

- Entity-Component-Systems (ECS) architecture for scene management
- Cross-platform support on platforms supporting OpenGL and GLFW
- Physically-based deferred rendering, tangent space normal mapping and global image-based lighting with the Cook-Torrance lighting model
- OOP based encapsulation of common OpenGL functionality (textures, framebuffers, shaders etc.)
- Transform-hierarchy for scene objects
- BVH for ray intersection queries
- Support for multiple cameras per scene, for e.g., to create split-screen apps, etc.

**RCubeViewer** is a higher level viewer API to quickly prototype lightweight interactive 3D applications for scientific visualization and research projects with minimum effort. RCubeViewer supports visualizing and inspecting the following geometry with a few lines of code:

- Pointclouds with scalar and vector fields
- Surface meshes with per-vertex, and per-face scalar and vector fields.

The example code below demonstrates how to visualize a pointcloud with `RCubeViewer`.
```cpp
#include "RCubeViewer/RCubeViewer.h"
#include "RCubeViewer/Pointcloud.h"

using namespace rcube;
using namespace rcube::viewer;

int main()
{
    // Create the viewer
    RCubeViewer v;

    // Create an entity to hold a pointcloud
    // RCubeViewer::createMeshEntity() automatically populates the entity with three components:
    // 1) A Drawable component that holds the mesh
    // 2) A Transform component that holds matrices for model-to-world transformation and possibly transform hierarchies
    // 3) A Material component to specify appearance properties in a physically-based manner
    // Any entity with these three components is rendered automatically by RCube's RenderSystem
    EntityHandle ent = v.createMeshEntity("my pointcloud");

    // Create the pointcloud data
    std::vector<glm::vec3> xyz_list = ...;
    auto pc = Pointcloud::create(xyz_list);

    // Set the pointcloud mesh in the entity's Drawable component
    ent.get<Drawable>()->mesh = pc;

    // Show the viewer
    v.execute();

    return 0;
}
```

More examples are available in the `examples` folder in the git repository.
