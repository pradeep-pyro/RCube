#pragma once

#include "RCubeViewer/Colormap.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
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
class ScalarField
{
    friend class Pointcloud;
    friend class SurfaceMesh;
    std::vector<glm::vec3> colors_;
    std::vector<float> data_;
    std::vector<float> histogram_;
    Colormap cmap_ = Colormap::Viridis;
    float vmin_ = 0.f;
    float vmax_ = 1.f;
    bool dirty_ = true;

  public:
    /**
     * Returns the scalar field data
     * @return const-ref to scalar field
     */
    const std::vector<float> &data() const;

    /**
     * Returns the scalar field data
     * @return mutable-ref to scalar field
     */
    std::vector<float> &data();

    /**
     * Sets the scalar field data
     * @param Scalar field as an array of floats
     */
    void setData(const std::vector<float> &data);

    /**
     * Returns the minimum data range of scalar field data
     * @return Minimum range of data
     */
    float dataMinRange() const;

    /**
     * Sets the minimum data range of scalar field data
     * @param Minimum range of data
     */
    void setDataMinRange(float val);

    /**
     * Returns the maximum data range of scalar field data
     * @return Miaxnimum range of data
     */
    float dataMaxRange() const;

    /**
     * Sets the minimum data range of scalar field data
     * @param Minimum range of data
     */
    void setDataMaxRange(float val);

    /**
     * Finds and sets the minimum and maximum data range from the scalar field data
     */
    void fitDataRange();

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
     * Updates the colors of the scalar field if necessary
     * Note: called by RCubeViewer internally
     *
     * @return Whether colors were actually updated
     */
    bool updateColors();

    /**
     * Update the histogram based on the current scalar field data
     * Note: called by RCubeViewer internally
     */
    void updateHistogram();
};

} // namespace viewer
} // namespace rcube