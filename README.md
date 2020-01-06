# RCube

RCube (for <img src="https://render.githubusercontent.com/render/math?math=\mathbb{R}^3">) is an experimental, easy to use OpenGL 4.2 engine for 3D graphics/visualization. It is a WIP.

Some features include:

- Entity-Component-Systems (ECS) architecture for scene management
- GLFW based window creation with simple event management
- Extensible materials including Blinn-Phong and physically-based (Cook Torrance+GGX) shaders
- Image-based lighting
- OOP based encapsulation of common OpenGL functionality (Texture, Framebuffer, ShaderProgram etc.)
- Transform-hierarchy for scene objects
- Multiple cameras per scene
- Triangle mesh generation for various primitives: box, sphere, supershapes etc.
- Extensible full-screen effects for postprocessing
- A viewer API for quickly prototyping 3D applications.

TODO:

- [ ] Frustum culling
- [ ] Order independent transparency
- [ ] Shadows
- [ ] Scene/mesh loader based on Assimp
