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
 * VectorField is a class to store a field of vector values that can be displayed on
 * a pointcloud, surface mesh, etc.
 */
class VectorField
{
    friend class Pointcloud;
    friend class SurfaceMesh;
    TriangleMeshData mesh_;
    TriangleMeshData glyph_;
    std::vector<glm::vec3> vectors_;
    std::vector<glm::vec3> points_;
    float max_length_ = 1.f;
    bool scale_by_magnitude_ = true;
    Colormap cmap_ = Colormap::None;
    bool dirty_ = true;

  public:
    VectorField();

    /**
     * Returns the vector field
     * @return const-ref to the vector field
     */
    const std::vector<glm::vec3> &vectors() const;

    /**
     * Returns the vector field
     * @return mutable-ref to the vector field
     */
    std::vector<glm::vec3> &vectors();

    /**
     * Sets the vectors of the vector field
     * @param Vector field as an array of glm::vec3
     */
    void setVectors(const std::vector<glm::vec3> &data);

    /**
     * Returns the points where each vector starts
     * @return const-ref to the points
     */
    const std::vector<glm::vec3> &points() const;

    /**
     * Returns the points where each vector starts
     * @return mutable-ref to the points
     */
    std::vector<glm::vec3> &points();

    /**
     * Sets the points where each vector starts
     * @param Points as an array of glm::vec3
     */
    void setPoints(const std::vector<glm::vec3> &data);

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
     */
    bool updateArrows();

    /**
     * Sets the glyph that will be used to represent the vector.
     * Must be pointing along positive Y-axis.
     *
     * @param Glyph mesh to represent arrows
     */
    void setGlyph(const TriangleMeshData &glyph);

    /**
     * Get the mesh data associated to the vector field
     *
     * @return Vector field mesh data
     */
    const TriangleMeshData &mesh() const;
};

} // namespace viewer
} // namespace rcube