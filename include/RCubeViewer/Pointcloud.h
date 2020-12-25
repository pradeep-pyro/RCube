#pragma once

#include "RCube/Core/Graphics/MeshGen/Points.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCubeViewer/ScalarField.h"
#include "imgui.h"
#include <unordered_map>

namespace rcube
{
namespace viewer
{
class Pointcloud : public Mesh
{
  public:
    /**
     * Enum to specify how each point is rendered
     */
    enum class PointcloudGlyph
    {
        Sphere,
        Box
    };

  private:
    size_t num_vertices_per_point_ = 0;
    size_t num_triangles_per_point_ = 0;
    size_t num_points_ = 0;
    std::unordered_map<std::string, ScalarField> scalar_fields_;
    std::string visible_scalar_field_ = "";
    std::unordered_map<std::string, VectorField> vector_fields_;

    Pointcloud(const std::vector<glm::vec3> &points, float point_size, PointcloudGlyph glyph);

  public:
    /**
     * Creates an OpenGL mesh to represent Pointcloud data
     *
     * @param points vector of glm::vec3 points
     * @param point_size Size of the sphere for each point
     * @param glyph Enum specifying how each point is rendered
     * @return Shared pointer to Pointcloud
     */
    static std::shared_ptr<Pointcloud> create(const std::vector<glm::vec3> &points,
                                              float point_size,
                                              PointcloudGlyph glyph = PointcloudGlyph::Box);

    /**
     * Returns the number of vertices used by the mesh to represent each point
     *
     * @return Number of vertices in the per-point mesh
     */
    size_t verticesPerPoint() const;

    /**
     * Returns the number of triangles used by the mesh to represent each point
     *
     * @return Number of vertices in the per - point mesh
     */
    size_t trianglesPerPoint() const;

    /**
     * Add a scalar field (list of per-point scalars) for the pointcloud
     *
     * @param name Name of the scalar field for retrieving it and displaying in the GUI
     * @param sf Scalarfield
     */
    void addScalarField(std::string name, const ScalarField &sf);

    /**
     * Get the scalar field given its name
     *
     * @param name Name of the scalar field
     * @return sf const-ref to Scalarfield
     */
    const ScalarField &scalarField(std::string name) const;

    /**
     * Get the scalar field given its name
     *
     * @param name Name of the scalar field
     * @return sf ref to Scalarfield
     */
    ScalarField &scalarField(std::string name);

    /**
     * Show the scalar field in the viewport
     *
     * @param name Name of the scalar field
     */
    void showScalarField(std::string name);

    /**
     * Hide all scalar fields in the viewport
     */
    void hideAllScalarFields();

    /**
     * Update the points in the pointcloud
     * Note: this may invalidate some of the associated fields
     *
     * @param points vector of glm::vec3 points
     * @param point_size Size of the sphere for each point
     */
    void updatePoints(const std::vector<glm::vec3> &points, float point_size);

    /**
     * Draws the GUI for this pointcloud
     * Note: called by RCubeViewer internally
     */
    virtual void drawGUI() override;
};

} // namespace viewer
} // namespace rcube
