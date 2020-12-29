#pragma once

#include "RCube/Core/Graphics/MeshGen/Circle.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "glm/gtc/constants.hpp"

namespace rcube
{

TriangleMeshData cone(float radius, float height, int radial_segments, float theta_start,
                      float theta_end, bool bottom_cap);

} // namespace rcube