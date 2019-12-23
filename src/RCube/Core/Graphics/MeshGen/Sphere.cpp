#include "RCube/Core/Graphics/MeshGen/Sphere.h"
#include "RCube/Core/Graphics/MeshGen/Box.h"
#include "glm/gtc/constants.hpp"

namespace rcube
{

MeshData icoSphere(float radius, unsigned int subdivisions)
{
    // Golden ratio
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    // Vertices of a icosahedron
    std::vector<glm::vec3> verts = {{-1, t, 0}, {1, t, 0}, {-1, -t, 0}, {1, -t, 0},
                                    {0, -1, t}, {0, 1, t}, {0, -1, -t}, {0, 1, -t},
                                    {t, 0, -1}, {t, 0, 1}, {-t, 0, -1}, {-t, 0, 1}};
    // Faces of the icosahedron
    std::vector<unsigned int> faces = {0, 11, 5, 0, 5,  1,  0,  1,  7,  0,  7, 10, 0, 10, 11,
                                       1, 5,  9, 5, 11, 4,  11, 10, 2,  10, 7, 6,  7, 1,  8,
                                       3, 9,  4, 3, 4,  2,  3,  2,  6,  3,  6, 8,  3, 8,  9,
                                       4, 9,  5, 2, 4,  11, 6,  2,  10, 8,  6, 7,  9, 8,  1};

    // Subdivision
    for (unsigned int sub = 0; sub < subdivisions; ++sub)
    {
        size_t orig_len = faces.size();
        for (size_t idx = 0; idx < orig_len; idx += 3)
        {
            unsigned int i = faces[idx];
            unsigned int j = faces[idx + 1];
            unsigned int k = faces[idx + 2];
            const glm::vec3 &a = verts[i];
            const glm::vec3 &b = verts[j];
            const glm::vec3 &c = verts[k];
            glm::vec3 ab = (a + b) / 2.f;
            glm::vec3 bc = (b + c) / 2.f;
            glm::vec3 ca = (c + a) / 2.f;
            verts.insert(verts.end(), {ab, bc, ca});
            unsigned int ij = verts.size() - 3;
            unsigned int jk = verts.size() - 2;
            unsigned int ki = verts.size() - 1;
            faces.insert(faces.end(), {i, ij, ki, ij, j, jk, ki, jk, k});
            faces[idx] = jk;
            faces[idx + 1] = ki;
            faces[idx + 2] = ij;
        }
    }

    // Make each vertex to lie on the sphere and find its normal
    std::vector<glm::vec3> normals;
    normals.reserve(verts.size());
    for (auto &v : verts)
    {
        auto norm_v = glm::normalize(v);
        normals.push_back(norm_v);
        v = norm_v * radius;
    }

    MeshData data;
    data.vertices = verts;
    data.indices = faces;
    data.normals = normals;
    data.indexed = true;

    return data;
}

MeshData cubeSphere(float radius, unsigned int n_segments)
{
    MeshData data = box(1, 1, 1, n_segments, n_segments, n_segments);
    assert(data.vertices.size() == data.normals.size());
    for (size_t i = 0; i < data.vertices.size(); ++i)
    {
        glm::vec3 norm_v = glm::normalize(data.vertices[i]);
        data.vertices[i] = norm_v * radius;
        data.normals[i] = norm_v;
    }
    return data;
}

} // namespace rcube
