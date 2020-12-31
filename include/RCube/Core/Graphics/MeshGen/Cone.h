#pragma once

#include "RCube/Core/Graphics/MeshGen/Circle.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "glm/gtc/constants.hpp"

namespace rcube
{

TriangleMeshData coneIndexed(float radius, float height, int radial_segments,
                             float theta_start = 0.f, float theta_end = glm::two_pi<float>(),
                             bool bottom_cap = true);

TriangleMeshData cone(float radius, float height, int radial_segments, float theta_start = 0.f,
                      float theta_end = glm::two_pi<float>(), bool bottom_cap = true);

} // namespace rcube