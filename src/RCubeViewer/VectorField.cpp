#include "RCubeViewer/VectorField.h"
#include "RCube/Core/Graphics/MeshGen/Cone.h"
#include "RCube/Core/Graphics/MeshGen/Points.h"
#include "RCubeViewer/Colormap.h"
#include "glm/gtx/quaternion.hpp"
#include <algorithm>
#include <string>
#include <vector>

namespace rcube
{
namespace viewer
{
VectorField::VectorField()
{
    glyph_ = cone(0.1f * 1, 1, 8, 0, glm::pi<float>() * 2.f, false);
}
const std::vector<glm::vec3> &VectorField::vectors() const
{
    return vectors_;
}
std::vector<glm::vec3> &VectorField::vectors()
{
    dirty_ = true;
    return vectors_;
}
void VectorField::setVectors(const std::vector<glm::vec3> &data)
{
    vectors_ = data;
    dirty_ = true;
}
const std::vector<glm::vec3> &VectorField::points() const
{
    return points_;
}
std::vector<glm::vec3> &VectorField::points()
{
    dirty_ = true;
    return points_;
}
void VectorField::setPoints(const std::vector<glm::vec3> &data)
{
    points_ = data;
    dirty_ = true;
}
float VectorField::maxLength() const
{
    return max_length_;
}
void VectorField::setMaxLength(float val)
{
    max_length_ = val;
    dirty_ = true;
}
Colormap VectorField::cmap() const
{
    return cmap_;
}
void VectorField::setCmap(Colormap cmap)
{
    cmap_ = cmap;
    dirty_ = true;
}
bool VectorField::updateArrows()
{
    // Don't recompute the arrows if nothing has changed
    if (dirty_)
    {
        assert(points_.size() == vectors.size());
        mesh_.clear();
        if (points_.empty())
        {
            dirty_ = false;
            return true;
        }
        std::vector<float> lengths;
        lengths.reserve(vectors_.size());
        for (const glm::vec3 &vec : vectors_)
        {
            const float length = glm::length(vec);
            lengths.push_back(length);
        }
        // Compute colors for arrows
        std::vector<glm::vec3> colors;
        auto minmax_length = std::minmax_element(lengths.begin(), lengths.end());
        colormap(cmap_, lengths, *minmax_length.first, *minmax_length.second, colors);

        // If scale_by_magnitude_ is true...
        if (!scale_by_magnitude_)
        {
            // ...then, set all vectors' length to max_length_
            std::fill(lengths.begin(), lengths.end(), max_length_);
        }
        else
        {
            // ...else, scale the vector lengths such that the longest is max_length_ long.
            // Find the longest vector's length
            float longest_length = *std::max_element(lengths.begin(), lengths.end());
            for (float &len : lengths)
            {
                len *= max_length_;
                // Make sure not to divide by 0
                if (longest_length > 1e-6)
                {
                    len = len / longest_length;
                }
            }
        }
        // Create the arrows mesh
        size_t vertices_per_arrow = glyph_.vertices.size();
        glm::vec3 mn, mx;
        glyph_.boundingBox(mn, mx);
        mesh_.indexed = glyph_.indexed;
        float glyph_length = std::abs(mx[1] - mn[1]);
        for (size_t i = 0; i < points_.size(); ++i)
        {
            TriangleMeshData curr_glyph = glyph_;
            // Create an arrow as a cone
            float h = lengths[i];
            float scale = h / glyph_length;
            for (glm::vec3 &vertex : curr_glyph.vertices)
            {
                vertex *= scale;
            }
            for (glm::vec3 &vertex : curr_glyph.vertices)
            {
                vertex[1] += 0.5f * h;
            }
            glm::quat q = glm::rotation(glm::vec3(0.f, 1.f, 0.f), glm::normalize(vectors_[i]));
            for (glm::vec3 &vertex : curr_glyph.vertices)
            {
                vertex = glm::rotate(q, vertex);
            }
            // Shift the arrow such that the vector's tail coincides with points[i]
            for (glm::vec3 &vertex : curr_glyph.vertices)
            {
                vertex += points_[i];
            }
            mesh_.append(curr_glyph);
        }
        // Set colors for arrows based on colormap
        // The color must be set for each of the vertices in the arrow's mesh
        for (size_t i = 0; i < colors.size(); ++i)
        {
            for (size_t j = 0; j < vertices_per_arrow; ++j)
            {
                mesh_.colors.push_back(colors[i]);
            }
        }
        dirty_ = false;
        return true;
    }
    return false;
}
void VectorField::setGlyph(const TriangleMeshData &glyph)
{
    glyph_ = glyph;
    dirty_ = true;
}
const TriangleMeshData &VectorField::mesh() const
{
    return mesh_;
}

} // namespace viewer
} // namespace rcube