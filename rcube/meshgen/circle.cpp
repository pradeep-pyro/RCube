#include "circle.h"

namespace rcube {

MeshData circle(float radius, int radial_segments, float theta_start, float theta_end) {
    MeshData data;
    data.indexed = true;
    data.primitive = MeshPrimitive::Triangles;

    data.vertices.reserve(radial_segments + 2);
    data.texcoords.reserve(radial_segments + 2);
    data.normals.reserve(radial_segments + 2);

    // Center
    glm::vec3 center(0);
    data.vertices.push_back(center);
    data.normals.push_back(glm::vec3(0, 1, 0));
    data.texcoords.push_back(glm::vec2(0.5f, 0.5f));

    float theta_inc = (theta_end - theta_start) / radial_segments;
    for (int i = 0; i <= radial_segments; ++i) {
        float curr_theta = theta_start + i * theta_inc;
        glm::vec3 vertex = radius * glm::vec3(std::cos(curr_theta), 0, std::sin(curr_theta));
        data.vertices.push_back(vertex);
        glm::vec2 uv;
        uv[0] = (vertex.x / radius + 1 ) / 2;
        uv[1] = (vertex.z / radius + 1 ) / 2;
        data.texcoords.push_back(uv);
        data.normals.push_back(glm::vec3(0, 1, 0));
    }

    for (int i = 0; i <= radial_segments; ++i) {
        data.indices.push_back(i);
        data.indices.push_back(i + 1);
        data.indices.push_back(0);
    }

    return data;
}

} // namespace rcube
