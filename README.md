# RCube

RCube (for R^3) is an experimental, easy to use OpenGL 4.2 engine for 3D graphics/visualization, mostly used personally for prototyping research projects. It is a WIP.

Some features include:

- Entity-Component-Systems (ECS) architecture for scene management
- GLFW based window creation with simple event management
- Extensible materials including Blinn-Phong and physically-based (Cook Torrance+GGX)
- Environment map prefiltering (currently supports diffuse only)
- OOP based encapsulation of common OpenGL functionality (Texture, Framebuffer, ShaderProgram etc.)
- Transform-hierarchy for scene objects
- Multiple cameras per scene
- Triangle mesh generation for various primitives: box, sphere, supershapes etc.
- Extensible full-screen effects for postprocessing

etc.

TODO:

- [ ] Frustum culling
- [ ] Order independent transparency
- [ ] Shadows
- [ ] Specular environment map prefiltering for IBL
- [ ] Scene loader based on Assimp
- [ ] Add examples
