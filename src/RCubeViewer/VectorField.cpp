#include "RCubeViewer/VectorField.h"
#include "RCube/Core/Graphics/MeshGen/Points.h"
#include "RCubeViewer/Colormap.h"
#include <algorithm>
#include <string>
#include <vector>

namespace rcube
{
namespace viewer
{

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
bool VectorField::updateArrows(const std::vector<glm::vec3> &points, bool indexed)
{
    // Don't recompute the arrows if nothing has changed
    if (dirty_)
    {
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
        size_t vertices_per_arrow;
        if (indexed)
        {
            size_t triangles_per_arrow;
            mesh_ = pointsVectorsToArrowsIndexed(points, vectors_, lengths, vertices_per_arrow,
                                                 triangles_per_arrow);
        }
        else
        {
            mesh_ = pointsVectorsToArrows(points, vectors_, lengths, vertices_per_arrow);
        }
        // Set colors for arrows based on colormap
        // The color must be set for each of the vertices in the arrow's mesh
        size_t k = 0;
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

} // namespace viewer
} // namespace rcube