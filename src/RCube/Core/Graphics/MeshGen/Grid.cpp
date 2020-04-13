#include "RCube/Core/Graphics/MeshGen/Grid.h"

namespace rcube
{

LineMeshData grid(float width, float height, int width_segments, int height_segments,
                  glm::vec3 color_centerline_x, glm::vec3 color_centerline_z, glm::vec3 color_grid)
{
    LineMeshData mesh;
    mesh.indexed = false;
    float half_width = width / 2.f;
    float half_height = height / 2.f;
    float edge_width = width / width_segments, edge_height = height / height_segments;

    int w_centerline = width_segments / 2;
    int h_centerline = height_segments / 2;

    for (int i = 0; i <= width_segments; ++i)
    {
        mesh.vertices.push_back(glm::vec3(-half_width + i * edge_width, 0, -half_height));
        mesh.vertices.push_back(glm::vec3(-half_width + i * edge_width, 0, half_height));
        mesh.colors.push_back((i == w_centerline) ? color_centerline_x : color_grid);
        mesh.colors.push_back((i == w_centerline) ? color_centerline_x : color_grid);
    }
    for (int i = 0; i <= height_segments; ++i)
    {
        mesh.vertices.push_back(glm::vec3(-half_width, 0, -half_height + i * edge_height));
        mesh.vertices.push_back(glm::vec3(half_width, 0, -half_height + i * edge_height));
        mesh.colors.push_back((i == h_centerline) ? color_centerline_z : color_grid);
        mesh.colors.push_back((i == h_centerline) ? color_centerline_z : color_grid);
    }
    return mesh;
}

} // namespace rcube
