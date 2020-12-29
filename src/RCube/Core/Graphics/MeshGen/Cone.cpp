#include "RCube/Core/Graphics/MeshGen/Cone.h"

namespace rcube
{

TriangleMeshData cone(float radius, float height, int radial_segments, float theta_start,
                      float theta_end, bool bottom_cap)
{
    float half_height = height / 2.f;
    float slope = radius / height;
    float theta_inc = (theta_end - theta_start) / radial_segments;

    TriangleMeshData cone = circle(radius, radial_segments, theta_start, theta_end);
    // Move the center point to the top and other points to the bottom
    cone.vertices[0].y = half_height;
    for (size_t i = 1; i < cone.vertices.size(); ++i)
    {
        cone.vertices[i].y = -half_height;
        float curr_theta = theta_start + (i - 1) * theta_inc;
        float cos_theta = std::cos(curr_theta);
        float sin_theta = std::sin(curr_theta);
        cone.normals[i] = glm::normalize(glm::vec3(cos_theta, slope, sin_theta));
    }

    return cone;
}

} // namespace rcube