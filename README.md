# RCube

RCube (for <img src="https://render.githubusercontent.com/render/math?math=\mathbb{R}^3">) is an experimental, easy to use OpenGL 4.5 engine for 3D graphics/visualization. It is a WIP.

## Features

- Entity-Component-Systems (ECS) architecture for scene management
- GLFW for window creation
- Physically-based deferred shading using the Cook Torrance microfacet distribution function
- Global image-based lighting
- OOP based encapsulation of common OpenGL functionality (textures, framebuffers, shaders etc.)
- Transform-hierarchy for scene objects
- Multiple cameras per scene
- Triangle mesh generation for various primitives: box, sphere, supershape etc.
- A viewer API for quickly prototyping 3D applications

## TODO

- [ ] Frustum culling
- [ ] Order independent transparency
- [ ] Shadows
- [ ] Scene/mesh loader based on Assimp

## Gallery

TODO


## References used

- Physically-based rendering: https://learnopengl.com/PBR/Theory
- Entity-Component-Systems architecture: https://medium.com/@savas/nomad-game-engine-part-1-why-3be9825cb90a
- Wireframe rendering: http://developer.download.nvidia.com/whitepapers/2007/SDK10/SolidWireframe.pdf
