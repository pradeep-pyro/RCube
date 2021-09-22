#include "RCube/Core/Graphics/MeshGen/Sphere.h"
#include "RCube/Core/Graphics/MeshGen/Box.h"
#include "glm/gtc/constants.hpp"

namespace rcube
{

TriangleMeshData icoSphere(float radius, unsigned int subdivisions)
{
    // Golden ratio
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    // Vertices of a icosahedron
    std::vector<glm::vec3> verts = {{-1, t, 0}, {1, t, 0}, {-1, -t, 0}, {1, -t, 0},
                                    {0, -1, t}, {0, 1, t}, {0, -1, -t}, {0, 1, -t},
                                    {t, 0, -1}, {t, 0, 1}, {-t, 0, -1}, {-t, 0, 1}};
    // Faces of the icosahedron
    std::vector<glm::uvec3> faces = {{0, 11, 5}, {0, 5, 1},  {0, 1, 7},   {0, 7, 10}, {0, 10, 11},
                                     {1, 5, 9},  {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
                                     {3, 9, 4},  {3, 4, 2},  {3, 2, 6},   {3, 6, 8},  {3, 8, 9},
                                     {4, 9, 5},  {2, 4, 11}, {6, 2, 10},  {8, 6, 7},  {9, 8, 1}};

    // Subdivision
    for (unsigned int sub = 0; sub < subdivisions; ++sub)
    {
        size_t orig_num_faces = faces.size();
        for (size_t idx = 0; idx < orig_num_faces; ++idx)
        {
            unsigned int i = faces[idx][0];
            unsigned int j = faces[idx][1];
            unsigned int k = faces[idx][2];
            const glm::vec3 &a = verts[i];
            const glm::vec3 &b = verts[j];
            const glm::vec3 &c = verts[k];
            glm::vec3 ab = (a + b) / 2.f;
            glm::vec3 bc = (b + c) / 2.f;
            glm::vec3 ca = (c + a) / 2.f;
            verts.insert(verts.end(), {ab, bc, ca});
            unsigned int ij = (unsigned int)(verts.size()) - 3;
            unsigned int jk = (unsigned int)(verts.size()) - 2;
            unsigned int ki = (unsigned int)(verts.size()) - 1;
            faces.insert(faces.end(), {{i, ij, ki}, {ij, j, jk}, {ki, jk, k}});
            faces[idx] = {jk, ki, ij};
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

    TriangleMeshData data;
    data.vertices = verts;
    data.indices = faces;
    data.normals = normals;
    data.indexed = true;

    return data;
}

TriangleMeshData cubeSphere(float radius, unsigned int n_segments)
{
    TriangleMeshData data = box(1, 1, 1, n_segments, n_segments, n_segments);
    assert(data.vertices.size() == data.normals.size());
    for (size_t i = 0; i < data.vertices.size(); ++i)
    {
        glm::vec3 norm_v = glm::normalize(data.vertices[i]);
        data.vertices[i] = norm_v * radius;
        data.normals[i] = norm_v;
    }
    return data;
}

TriangleMeshData uvSphere(float radius, unsigned int long_segments, unsigned int lat_segments)
{
    // Based on http://www.songho.ca/opengl/gl_sphere.html
    TriangleMeshData data;
    data.indexed = true;
    float lat_step = glm::two_pi<float>() / lat_segments;
    float long_step = glm::pi<float>() / long_segments;
    float inv_radius = 1.0f / radius;
    for (unsigned int i = 0; i <= long_segments; ++i)
    {
        float long_angle = glm::half_pi<float>() - i * long_step; // starting from pi/2 to -pi/2
        float xy = radius * std::cos(long_angle);                 // r * cos(u)
        float z = radius * std::sin(long_angle);                  // r * sin(u)

        for (unsigned int j = 0; j <= lat_segments; ++j)
        {
            float lat_angle = j * lat_step; // starting from 0 to 2pi

            float x = xy * std::cos(lat_angle); // r * cos(u) * cos(v)
            float y = xy * std::sin(lat_angle); // r * cos(u) * sin(v)
            data.vertices.push_back({x, z, y});

            float nx = x * inv_radius;
            float ny = z * inv_radius;
            float nz = y * inv_radius;
            data.normals.push_back({nx, ny, nz});

            float u = (float)j / lat_segments;
            float v = (float)i / long_segments;
            data.texcoords.push_back({u, v});
        }
    }

    for (unsigned int i = 0; i < long_segments; ++i)
    {
        unsigned int k1 = i * (lat_segments + 1); // beginning of current stack
        unsigned int k2 = k1 + lat_segments + 1;  // beginning of next stack

        for (unsigned int j = 0; j < lat_segments; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                data.indices.push_back({k1, k2, k1 + 1});
            }

            // k1+1 => k2 => k2+1
            if (i != (long_segments - 1))
            {
                data.indices.push_back({k1 + 1, k2, k2 + 1});
            }
        }
    }
    return data;
}

} // namespace rcube
