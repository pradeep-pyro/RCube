#pragma once

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "glm/glm.hpp"
#include <vector>

namespace rcube
{

TriangleMeshData pointsToSpheres(const std::vector<glm::vec3> &points,
                                 const std::vector<float> &radius,
                                 const std::vector<glm::vec3> &colors,
                                 size_t &num_triangles_per_point);

TriangleMeshData pointsToSpheres(const std::vector<glm::vec3> &points, float radius, size_t &num_vertices_per_point, size_t &num_triangles_per_point);

TriangleMeshData pointsToBoxes(const std::vector<glm::vec3> &points, float side,
                               size_t &num_triangles_per_point);

} // namespace rcube