#pragma once

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCubeViewer/ScalarField.h"
#include "RCubeViewer/VectorField.h"
#include "imgui.h"
#include <unordered_map>

namespace rcube
{
namespace viewer
{

/**
 * Pointcloud is a class that derives from Mesh and provides an API
 * to automatically create a mesh from a pointcloud (list of points).
 * It generates a mesh based on per-point glyphs (spheres or boxes)
 * and allows setting scalar and vector valued per-point attributes.
 */
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
    std::vector<glm::vec3> points_;
    float point_size_ = 0.01f;
    glm::vec3 color_ = glm::vec3(1.f, 1.f, 1.f);
    PointcloudGlyph glyph_;
    size_t num_vector_field_vertices_ = 0;
    size_t num_vector_field_faces_ = 0;
    std::unordered_map<std::string, ScalarField> scalar_fields_;
    std::string visible_scalar_field_ = "(None)";
    std::unordered_map<std::string, VectorField> vector_fields_;
    std::string visible_vector_field_ = "(None)";

    Pointcloud(const std::vector<glm::vec3> &points, float point_size, PointcloudGlyph glyph);

    void setPointcloudColorAttribute(const std::vector<glm::vec3> &perPointColors);
    void setPointcloudColorAttribute(const glm::vec3 &perPointColor);
    void setPointcloudArrowAttributes(const TriangleMeshData &mesh);
    void createMesh();

  public:
    /**
     * Creates an OpenGL mesh to represent Pointcloud data
     *
     * @param points vector of glm::vec3 points
     * @param point_size Size of the sphere for each point
     * @param glyph Enum specifying how each point is rendered
     * @return Shared pointer to rcube::viewer::Pointcloud (derived from rcube::Mesh)
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
     * @return Number of triangles in the per-point mesh
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
     * Removes the scalar field
     *
     * @param name Name of the scalar field
     */
    void removeScalarField(std::string name);

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
     * Add a vector field (list of per-point vectors) for the pointcloud
     *
     * @param name Name of the vector field for retrieving it and displaying in the GUI
     * @param vf Vectorfield
     */
    void addVectorField(std::string name, const VectorField &vf);

    /**
     * Removes the vector field
     *
     * @param name Name of the vector field
     */
    void removeVectorField(std::string name);

    /**
     * Get the vector field given its name
     *
     * @param name Name of the vector field
     * @return const-ref to Vectorfield
     */
    const VectorField &vectorField(std::string name) const;

    /**
     * Get the vector field given its name
     *
     * @param name Name of the vector field
     * @return ref to Vectorfield
     */
    VectorField &vectorField(std::string name);

    /**
     * Show the vector field in the viewport
     *
     * @param name Name of the vector field
     */
    void showVectorField(std::string name);

    /**
     * Hide all vector fields in the viewport
     */
    void hideAllVectorFields();

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

    /**
     * Returns the number of points in the point cloud
     *
     * @return Number of points
     */
    size_t numPoints() const;

    /**
     * Returns the size of each rendered point in the point cloud
     *
     * @return Size of each point
     */
    float pointSize() const;

    /**
     * Returns the color of the pointcloud
     *
     * @return Color of the pointcloud
     */
    glm::vec3 color() const;

    /**
     * Sets the color of the pointcloud
     *
     * @param Color of the pointcloud
     */
    void setColor(const glm::vec3 &col);
};

} // namespace viewer
} // namespace rcube
