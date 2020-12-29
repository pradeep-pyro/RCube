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

class SurfaceMesh : public Mesh
{
    glm::vec3 color_ = glm::vec3(1.f, 1.f, 1.f);
    std::unordered_map<std::string, ScalarField> scalar_fields_;
    std::string visible_scalar_field_ = "(None)";
    std::unordered_map<std::string, VectorField> vertex_vector_fields_;
    std::string visible_vertex_vector_field_ = "(None)";
    std::unordered_map<std::string, VectorField> face_vector_fields_;
    std::string visible_face_vector_field_ = "(None)";
    std::vector<glm::vec3> vertices_;
    std::vector<glm::vec3> face_centers_;
    size_t num_vertex_vector_field_vertices_ = 0;
    size_t num_vertex_vector_field_triangles_ = 0;
    size_t num_face_vector_field_vertices_ = 0;
    size_t num_face_vector_field_triangles_ = 0;

    void createMesh(const TriangleMeshData &data);

    SurfaceMesh(const TriangleMeshData &data);

    void setVertexColorAttribute(const glm::vec3 &perPointColor);
    void setVertexColorAttribute(const std::vector<glm::vec3> &perPointColors);
    void setVertexArrowMesh(const TriangleMeshData &mesh);
    void setFaceArrowMesh(const TriangleMeshData &mesh);

  public:
    static std::shared_ptr<SurfaceMesh> create(const TriangleMeshData &data);

    /**
     * Add a scalar field (list of per-vertex scalars) for the surface mesh
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
     * Add a vector field (list of per-vertex vectors) for the surface mesh
     *
     * @param name Name of the vector field for retrieving it and displaying in the GUI
     * @param vf Vectorfield
     */
    void addVertexVectorField(std::string name, const VectorField &vf);

    /**
     * Removes the vertex vector field
     *
     * @param name Name of the vector field
     */
    void removeVertexVectorField(std::string name);

    /**
     * Add a vector field (list of per-face vectors) for the surface mesh
     *
     * @param name Name of the vector field for retrieving it and displaying in the GUI
     * @param vf Vectorfield
     */
    void addFaceVectorField(std::string name, const VectorField &vf);

    /**
     * Removes the face vector field
     *
     * @param name Name of the vector field
     */
    void removeFaceVectorField(std::string name);

    /**
     * Get the vertex vector field given its name
     *
     * @param name Name of the vector field
     * @return const-ref to Vectorfield
     */
    const VectorField &vertexVectorField(std::string name) const;

    /**
     * Get the vertex vector field given its name
     *
     * @param name Name of the vector field
     * @return ref to Vectorfield
     */
    VectorField &vertexVectorField(std::string name);

    /**
     * Get the face vector field given its name
     *
     * @param name Name of the vector field
     * @return const-ref to Vectorfield
     */
    const VectorField &faceVectorField(std::string name) const;

    /**
     * Get the face vector field given its name
     *
     * @param name Name of the vector field
     * @return ref to Vectorfield
     */
    VectorField &faceVectorField(std::string name);

    /**
     * Show the vertex vector field in the viewport
     *
     * @param name Name of the vector field
     */
    void showVertexVectorField(std::string name);

    /**
     * Show the face vector field in the viewport
     *
     * @param name Name of the vector field
     */
    void showFaceVectorField(std::string name);

    /**
     * Hide all vertex vector fields in the viewport
     */
    void hideAllVertexVectorFields();

    /**
     * Hide all face vector fields in the viewport
     */
    void hideAllFaceVectorFields();

    /**
     * Draws the GUI for this pointcloud
     * Note: called by RCubeViewer internally
     */
    virtual void drawGUI() override;

    glm::vec3 color() const;

    void setColor(const glm::vec3 &col);

    size_t numVertices() const;

    size_t numFaces() const;

    virtual void updateBVH() override;
};

} // namespace viewer
} // namespace rcube