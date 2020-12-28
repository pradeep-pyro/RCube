#pragma once

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCubeViewer/Colormap.h"
#include <string>
#include <vector>

namespace rcube
{
namespace viewer
{

/**
 * ScalarField is a class to store a field of scalar values that can be displayed on
 * a pointcloud, surface mesh, etc.
 */

class VectorField
{
    friend class Pointcloud;
    TriangleMeshData mesh_;
    std::vector<glm::vec3> vectors_;
    float max_length_ = 0.01f;
    bool scale_by_magnitude_ = true;
    Colormap cmap_ = Colormap::Viridis;
    bool dirty_ = true;

    static TriangleMeshData createArrowMesh();

  public:
    /**
     * Returns the vector field
     * @return const-ref to the vector field
     */
    const std::vector<glm::vec3> &vectors() const;

    /**
     * Sets the vectors of the vector field
     * @param Vector field as an array of glm::vec3
     */
    void setVectors(const std::vector<glm::vec3> &data);

    /**
     * Returns the maximum length to which the longest vector in the vector field
     * will be scaled (default: 1).
     *
     * @return Maximum length to scale vector data
     */
    float maxLength() const;

    /**
     * Sets the maximum length to which the longest vector in the vector field
     * will be scaled.
     *
     * @param Maximum length to scale vector data
     */
    void setMaxLength(float val);

    /**
     * Returns the current colormap
     * @return Colormap enum
     */
    Colormap cmap() const;

    /**
     * Sets the colormap type
     * @param Colormap enum
     */
    void setCmap(Colormap cmap);

    /**
     * Updates the arrows of the vector field if necessary
     * Note: called by RCubeViewer internally
     */
    bool updateArrows(const std::vector<glm::vec3> &points);
};

} // namespace viewer
} // namespace rcube