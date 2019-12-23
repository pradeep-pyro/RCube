#include "RCube/Core/Graphics/MeshGen/Box.h"
#include "RCube/Core/Graphics/MeshGen/Plane.h"

namespace rcube
{

MeshData box(float width, float height, float depth, unsigned int width_segments,
             unsigned int height_segments, unsigned int depth_segments)
{
    std::vector<MeshData> data;
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
    MeshData box_data;
    box_data.indexed = true;
    box_data.primitive = MeshPrimitive::Triangles;
    box_data.vertices.reserve(n_verts);
    box_data.normals.reserve(n_verts);
    box_data.texcoords.reserve(n_verts);
    box_data.indices.reserve(n_indices);
    for (int i = 0; i < 6; ++i)
    {
        for (auto &ind : data[i].indices)
        {
            ind += offset[i];
        }
        box_data.vertices.insert(box_data.vertices.end(), data[i].vertices.begin(),
                                 data[i].vertices.end());
        box_data.indices.insert(box_data.indices.end(), data[i].indices.begin(),
                                data[i].indices.end());
        box_data.normals.insert(box_data.normals.end(), data[i].normals.begin(),
                                data[i].normals.end());
        box_data.texcoords.insert(box_data.texcoords.end(), data[i].texcoords.begin(),
                                  data[i].texcoords.end());
    }
    return box_data;
}

} // namespace rcube
