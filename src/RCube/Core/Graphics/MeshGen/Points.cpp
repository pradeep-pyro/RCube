#include "RCube/Core/Graphics/MeshGen/Sphere.h"
#include "RCube/Core/Graphics/MeshGen/Box.h"
#include "RCube/Core/Graphics/MeshGen/Points.h"

namespace rcube
{

TriangleMeshData pointsToSpheres(const std::vector<glm::vec3> &points,
                                 const std::vector<float> &radius,
                                 const std::vector<glm::vec3> &colors,
                                 size_t &num_triangles_per_point)
{
    TriangleMeshData data;
    data.indexed = true;
    for (size_t i = 0; i < points.size(); ++i)
    {
        TriangleMeshData sphere = icoSphere(radius[i], 1);
        for (glm::vec3 &vertex : sphere.vertices)
        {
            vertex += points[i];
        }
        num_triangles_per_point = sphere.indices.size();
        sphere.colors.resize(sphere.vertices.size(), colors[i]);
        data.append(sphere);
    }
    return data;
}

TriangleMeshData pointsToSpheres(const std::vector<glm::vec3> &points, float radius,
                                 size_t &num_triangles_per_point)
{
    TriangleMeshData data;
    data.indexed = true;
    for (size_t i = 0; i < points.size(); ++i)
    {
        TriangleMeshData sphere = icoSphere(radius, 1);
        for (glm::vec3 &vertex : sphere.vertices)
        {
            vertex += points[i];
        }
        data.append(sphere);
        num_triangles_per_point = sphere.indices.size();
    }
    return data;
}

TriangleMeshData pointsToBoxes(const std::vector<glm::vec3> &points, float side,
                               size_t &num_triangles_per_point)
{
    TriangleMeshData data;
    data.indexed = true;
    for (size_t i = 0; i < points.size(); ++i)
    {
        TriangleMeshData box_mesh = box(side, side, side, 1, 1, 1);
        for (glm::vec3 &vertex : box_mesh.vertices)
        {
            vertex += points[i];
        }
        num_triangles_per_point = box_mesh.indices.size();
        data.append(box_mesh);
    }
    return data;
}

} // namespace rcube
