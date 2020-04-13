#include "RCube/Core/Graphics/MeshGen/Box.h"
#include "RCube/Core/Graphics/MeshGen/Plane.h"

namespace rcube
{

TriangleMeshData box(float width, float height, float depth, unsigned int width_segments,
                     unsigned int height_segments, unsigned int depth_segments)
{
    std::vector<TriangleMeshData> data;
    data.resize(6);
    data[0] = plane(depth, height, depth_segments, height_segments, Orientation::PositiveX); // +x
    for (auto &v : data[0].vertices)
    {
        v.x += 0.5f * width;
    }
    data[1] = plane(depth, height, depth_segments, height_segments, Orientation::NegativeX); // -x
    for (auto &v : data[1].vertices)
    {
        v.x -= 0.5f * width;
    }
    data[2] = plane(width, depth, width_segments, depth_segments, Orientation::PositiveY); // +y
    for (auto &v : data[2].vertices)
    {
        v.y += 0.5f * height;
    }
    data[3] = plane(width, depth, width_segments, depth_segments, Orientation::NegativeY); // -y
    for (auto &v : data[3].vertices)
    {
        v.y -= 0.5f * height;
    }
    data[4] = plane(width, height, width_segments, height_segments, Orientation::PositiveZ); // +z
    for (auto &v : data[4].vertices)
    {
        v.z += 0.5f * depth;
    }
    data[5] = plane(width, height, width_segments, height_segments, Orientation::NegativeZ); // -z
    for (auto &v : data[5].vertices)
    {
        v.z -= 0.5f * depth;
    }

    size_t offset[6];
    offset[0] = 0;
    for (int i = 1; i < 6; ++i)
    {
        offset[i] = offset[i - 1] + data[i].vertices.size();
    }

    size_t n_verts = 0, n_indices = 0;
    for (int i = 0; i < 6; ++i)
    {
        n_verts += data[i].vertices.size();
        n_indices += data[i].indices.size();
    }
    TriangleMeshData box_data;
    box_data.indexed = true;
    for (size_t i = 0; i < 6; ++i)
    {
        box_data.append(data[i]);
    }
    return box_data;
}

} // namespace rcube
