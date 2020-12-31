#include "RCube/Core/Graphics/MeshGen/Cone.h"
#include "glm/gtx/string_cast.hpp"

namespace rcube
{

TriangleMeshData coneIndexed(float radius, float height, int radial_segments, float theta_start,
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

TriangleMeshData cone(float radius, float height, int radial_segments,
          float theta_start, float theta_end, bool bottom_cap)
{
    assert(theta_start < theta_end);
    float half_height = height / 2.f;
    float slope = radius / height;
    float theta_inc = (theta_end - theta_start) / radial_segments;

    TriangleMeshData data;
    data.clear();
    data.indexed = false;
    data.vertices.reserve(size_t(radial_segments) * (bottom_cap ? 4 : 3));
    data.normals.reserve(size_t(radial_segments) * (bottom_cap ? 4 : 3));

    for (size_t i = 0; i < radial_segments; ++i)
    {
        float curr_theta = theta_start + i * theta_inc;
        float curr_cos_theta = std::cos(curr_theta);
        float curr_sin_theta = std::sin(curr_theta);
        data.vertices.push_back(
            glm::vec3(radius * curr_cos_theta, -half_height, radius * curr_sin_theta));
        data.normals.push_back(glm::normalize(glm::vec3(curr_cos_theta, slope, curr_sin_theta)));

        float next_theta = theta_start + ((i + 1) % radial_segments) * theta_inc;
        float next_cos_theta = std::cos(next_theta);
        float next_sin_theta = std::sin(next_theta);
        data.vertices.push_back(
            glm::vec3(radius * next_cos_theta, -half_height, radius * next_sin_theta));
        data.normals.push_back(glm::normalize(glm::vec3(next_cos_theta, slope, next_sin_theta)));

        data.vertices.push_back(glm::vec3(0, half_height, 0));
        data.normals.push_back(glm::normalize(glm::vec3(0, 1, 0)));

        if (bottom_cap)
        {
            data.vertices.push_back(
                glm::vec3(radius * curr_cos_theta, -half_height, radius * curr_sin_theta));
            data.normals.push_back(glm::vec3(0, -1, 0));
            data.vertices.push_back(
                glm::vec3(radius * next_cos_theta, -half_height, radius * next_sin_theta));
            data.normals.push_back(glm::vec3(0, -1, 0));
            data.vertices.push_back(glm::vec3(0, -half_height, 0));
            data.normals.push_back(glm::vec3(0, -1, 0));
        }
    }
    return data;
}

} // namespace rcube