# RCube

RCube (for R^3) is an easy to use OpenGL 4.3 engine for 3D visualization, mostly used personally for prototyping research projects.
Some main features include:

- Entity-Component-Systems (ECS) architecture
- GLFW based window creation with simple event management
- Physically-based (Cook Torrance+GGX) shading with Image-Based Lighting (IBL)
- OOP based encapsulation of common OpenGL functionality (Texture, Framebuffer, ShaderProgram etc.)
- Transform-hierarchy for all scene objects
- Frustrum culling
- Full-screen effects for postprocessing

Todo:

- Order Independent Transparency
- Split-sum approximation for specular IBL
- Assimp based scene loader
- Shadows
