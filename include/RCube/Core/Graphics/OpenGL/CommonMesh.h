#pragma once

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "glm/glm.hpp"
#include <vector>

namespace rcube
{
namespace common
{
/**
 * Creates a skybox mesh that is compatible with the rcube::common::skyboxShader() shader.
 */
std::shared_ptr<Mesh> skyboxMesh();

/**
 * Creates a fullscreen quad mesh that is compatible with the rcube::common::fullscreenQuadShader() shader.
 */
std::shared_ptr<Mesh> fullScreenQuadMesh();

} // namespace common
} // namespace rcube