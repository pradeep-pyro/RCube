#include "RCube/Core/Graphics/MeshGen/Sphere.h"
#include "RCube/Core/Graphics/MeshGen/Box.h"
#include "RCube/Core/Graphics/MeshGen/Cylinder.h"
#include "RCube/Core/Graphics/MeshGen/Points.h"
#include "glm/gtx/quaternion.hpp"

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
                                 size_t &num_vertices_per_point, size_t &num_triangles_per_point)
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
        num_vertices_per_point = sphere.vertices.size();
        num_triangles_per_point = sphere.indices.size();
    }
    return data;
}

TriangleMeshData pointsToBoxes(const std::vector<glm::vec3> &points, float side,
                               size_t &num_vertices_per_point, size_t &num_triangles_per_point)
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
        num_vertices_per_point = box_mesh.vertices.size();
        num_triangles_per_point = box_mesh.indices.size();
        data.append(box_mesh);
    }
    return data;
}

TriangleMeshData pointsVectorsToArrows(const std::vector<glm::vec3> &points,
                                       const std::vector<glm::vec3> &vectors,
                                       const std::vector<float> &lengths,
                                       size_t &num_vertices_per_point,
                                       size_t &num_triangles_per_point)
{
    assert(points.size() == vectors.size());
    assert(points.size() == lengths.size());
    TriangleMeshData data;
    data.indexed = true;
    for (size_t i = 0; i < points.size(); ++i)
    {
        // Create an arrow as a cone
        float h = lengths[i];
        TriangleMeshData arrow_mesh =
            cylinder(0.1f * h, 0, h, 10, 1, 0, glm::pi<float>() * 2.f, false, true);
        // Rotate the arrow from y-axis to vector orientation
        glm::quat q = glm::rotation(glm::vec3(0.f, 1.f, 0.f), vectors[i]);
        // Shift the arrow such that the vector's tail is at origin
        for (glm::vec3 &vertex : arrow_mesh.vertices)
        {
            vertex.y += 0.5f * h;
        }
        for (glm::vec3 &vertex : arrow_mesh.vertices)
        {
            vertex = glm::rotate(q, vertex);
        }
        // Shift the arrow such that the vector's tail coincides with points[i]
        for (glm::vec3 &vertex : arrow_mesh.vertices)
        {
            vertex += points[i];
        }
        num_vertices_per_point = arrow_mesh.vertices.size();
        num_triangles_per_point = arrow_mesh.indices.size();
        data.append(arrow_mesh);
    }
    return data;
}

} // namespace rcube
