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
    friend class PickTooltipSystem;

    glm::vec3 color_ = glm::vec3(1.f, 1.f, 1.f);
    std::unordered_map<std::string, ScalarField> vertex_scalar_fields_;
    std::string visible_vertex_scalar_field_ = "(None)";
    std::unordered_map<std::string, ScalarField> face_scalar_fields_;
    std::string visible_face_scalar_field_ = "(None)";
    std::unordered_map<std::string, VectorField> vertex_vector_fields_;
    std::string visible_vertex_vector_field_ = "(None)";
    std::unordered_map<std::string, VectorField> face_vector_fields_;
    std::string visible_face_vector_field_ = "(None)";
    std::vector<glm::vec3> vertices_;
    std::vector<glm::uvec3> faces_;
    std::vector<glm::vec3> vertices_display_;
    std::vector<glm::vec3> face_centers_;
    size_t num_vertex_vector_field_vertices_ = 0;
    size_t num_face_vector_field_vertices_ = 0;
    std::unordered_map<size_t, glm::vec3> selected_face_colors_;

    void createMesh(const TriangleMeshData &data);

    SurfaceMesh(const TriangleMeshData &data);

    void setVertexColorAttribute(const glm::vec3 &vertex_color);
    void setVertexColorAttribute(const std::vector<glm::vec3> &vertex_colors);
    void setFaceColorAttribute(const glm::vec3 &face_color);
    void setFaceColorAttribute(const std::vector<glm::vec3> &face_colors);
    void setVertexArrowMesh(const TriangleMeshData &mesh);
    void setFaceArrowMesh(const TriangleMeshData &mesh);
    size_t numVerticesDisplay() const;

  public:
    static std::shared_ptr<SurfaceMesh> create(const TriangleMeshData &data);

    /**
     * Add a vertex scalar field (list of per-vertex scalars) for the surface mesh
     *
     * @param name Name of the scalar field for retrieving it and displaying in the GUI
     * @param sf Scalarfield
     */
    void addVertexScalarField(std::string name, const ScalarField &sf);

    /**
     * Removes the vertex scalar field
     *
     * @param name Name of the vertex scalar field
     */
    void removeVertexScalarField(std::string name);

    /**
     * Get the vertex scalar field given its name
     *
     * @param name Name of the vertex scalar field
     * @return sf const-ref to Scalarfield
     */
    const ScalarField &vertexScalarField(std::string name) const;

    /**
     * Get the vertex scalar field given its name
     *
     * @param name Name of the vertex scalar field
     * @return sf ref to Scalarfield
     */
    ScalarField &vertexScalarField(std::string name);

    /**
     * Show the vertex scalar field in the viewport
     *
     * @param name Name of the scalar field
     */
    void showVertexScalarField(std::string name);

    /**
     * Hide all vertex scalar fields in the viewport
     */
    void hideAllScalarFields();

    /**
     * Add a face scalar field (list of per-vertex scalars) for the surface mesh
     *
     * @param name Name of the scalar field for retrieving it and displaying in the GUI
     * @param sf Scalarfield
     */
    void addFaceScalarField(std::string name, const ScalarField &sf);

    /**
     * Removes the face scalar field
     *
     * @param name Name of the vertex scalar field
     */
    void removeFaceScalarField(std::string name);

    /**
     * Get the face scalar field given its name
     *
     * @param name Name of the vertex scalar field
     * @return sf const-ref to Scalarfield
     */
    const ScalarField &faceScalarField(std::string name) const;

    /**
     * Get the face scalar field given its name
     *
     * @param name Name of the vertex scalar field
     * @return sf ref to Scalarfield
     */
    ScalarField &faceScalarField(std::string name);

    /**
     * Show the face scalar field in the viewport
     *
     * @param name Name of the scalar field
     */
    void showFaceScalarField(std::string name);

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

    /**
     * Returns the color of the mesh
     * @return RGB color
     */
    glm::vec3 color() const;

    /**
     * Sets the color of the mesh
     * NOTE: this can also be set from the Material, but doing so will multiply the color with every
     * per-vertex color. This may tint/shade some of the scalar fields.
     *
     * @param RGB color
     */
    void setColor(const glm::vec3 &col);

    /**
     * Gives the number of vertices in the mesh
     *
     * @param Number of vertices
     */
    size_t numVertices() const;

    /**
     * Gives the number of faces in the mesh
     *
     * @param Number of faces
     */
    size_t numFaces() const;

    /**
     * Makes the given face appear to be selected by shading its color
     * Internally the original color (e.g. from scalar fields) are kept track of,
     * and will be restored when the face is unselected.
     *
     * @param Face index
     */
    void selectFace(size_t index);

    /**
     * Makes the given face appear to be unselected by restoring its original color
     *
     * @param Face index
     */
    void unselectFace(size_t index);
    
    /**
     * Makes all faces appear to be unselected by restoring their original color
     */
    void unselectFaces();
};

} // namespace viewer
} // namespace rcube