#include "RCubeViewer/SurfaceMesh.h"
#include "RCube/Core/Graphics/MeshGen/Points.h"
#include "imgui.h"

namespace rcube
{
namespace viewer
{
void SurfaceMesh::createMesh(const TriangleMeshData &data)
{
    vertices_ = data.vertices;
    face_centers_.reserve(data.indices.size());
    for (const glm::uvec3 &f : data.indices)
    {
        face_centers_.push_back((vertices_[f[0]] + vertices_[f[1]] + vertices_[f[2]]) / 3.f);
    }
    // The mesh attributes are setup such that the vertices of the surface come first,
    // followed by the vertices of the vertex vector field, followed by the vertices
    // of the face vector field
    // First, find the number of vertices needed for the arrow meshes
    size_t num_arrow_vertices, num_arrow_triangles;
    TriangleMeshData dummy_arrow = pointsVectorsToArrows(
        {glm::vec3(0, 0, 0)}, {glm::vec3(0, 1, 0)}, {1.f}, num_arrow_vertices, num_arrow_triangles);
    // Total number of vertices for the vertex vector field is num_arrow_vertices multiplied
    // by number of vertices
    num_vertex_vector_field_vertices_ = num_arrow_vertices * numVertices();
    num_vertex_vector_field_triangles_ = num_arrow_triangles * numVertices();
    // Similarly, number of vertices for face vector field is num_arrow_vertices multiplied
    // by number of faces
    num_face_vector_field_vertices_ = num_arrow_vertices * numFaces();
    num_face_vector_field_triangles_ = num_arrow_triangles * numFaces();

    attributes_["positions"]->data().resize(3 * (data.vertices.size() +
                                                 num_vertex_vector_field_vertices_ +
                                                 num_face_vector_field_vertices_),
                                            0.f);
    attributes_["normals"]->data().resize(3 * (data.vertices.size() +
                                               num_vertex_vector_field_vertices_ +
                                               num_face_vector_field_vertices_),
                                          0.f);
    attributes_["colors"]->data().resize(3 * (data.vertices.size() +
                                              num_vertex_vector_field_vertices_ +
                                              num_face_vector_field_vertices_),
                                         0.f);
    glm::vec3 *pos = attributes_["positions"]->ptrVec3();
    glm::vec3 *nor = attributes_["normals"]->ptrVec3();
    glm::vec3 *col = attributes_["colors"]->ptrVec3();
    for (size_t i = 0; i < data.vertices.size(); ++i)
    {
        pos[i] = data.vertices.at(i);
        nor[i] = data.normals.at(i);
        col[i] = color_;
    }
    indices_->data().resize(3 * (data.indices.size() + num_vertex_vector_field_triangles_ +
                                 num_face_vector_field_triangles_),
                            0);
    glm::uvec3 *ind = indices_->ptrUVec3();
    for (size_t i = 0; i < data.indices.size(); ++i)
    {
        ind[i] = data.indices.at(i);
    }

    updateBVH();
    uploadToGPU();
}

SurfaceMesh::SurfaceMesh(const TriangleMeshData &data)
    : Mesh({AttributeBuffer::create("positions", GLuint(AttributeLocation::POSITION), 3),
            AttributeBuffer::create("normals", GLuint(AttributeLocation::NORMAL), 3),
            AttributeBuffer::create("colors", GLuint(AttributeLocation::COLOR), 3)},
           MeshPrimitive::Triangles, true)
{
    createMesh(data);
}

std::shared_ptr<SurfaceMesh> SurfaceMesh::create(const TriangleMeshData &data)
{
    return std::shared_ptr<SurfaceMesh>(new SurfaceMesh(data));
}

void SurfaceMesh::setVertexColorAttribute(const std::vector<glm::vec3> &perPointColors)
{
    assert(perPointColors.size() == numPoints());
    glm::vec3 *colors = attributes_["colors"]->ptrVec3();
    for (size_t i = 0; i < numVertices(); ++i)
    {
        colors[i] = perPointColors[i];
    }
    uploadToGPU("colors");
}

void SurfaceMesh::setVertexColorAttribute(const glm::vec3 &perPointColor)
{
    glm::vec3 *colors = attributes_["colors"]->ptrVec3();
    for (size_t i = 0; i < numVertices(); ++i)
    {
        colors[i] = perPointColor;
    }
    uploadToGPU("colors");
}

size_t SurfaceMesh::numVertices() const
{
    return vertices_.size();
}

size_t SurfaceMesh::numFaces() const
{
    return face_centers_.size();
}

void SurfaceMesh::addScalarField(std::string name, const ScalarField &sf)
{
    scalar_fields_[name] = sf;
}

void SurfaceMesh::removeScalarField(std::string name)
{
    scalar_fields_.erase(name);
}

const ScalarField &SurfaceMesh::scalarField(std::string name) const
{
    return scalar_fields_.at(name);
}

ScalarField &SurfaceMesh::scalarField(std::string name)
{
    return scalar_fields_.at(name);
}

void SurfaceMesh::showScalarField(std::string name)
{
    ScalarField &sf = scalarField(name);
    // Update the scalar field if it was updated or
    // the selected one is different from what's being displayed
    if (sf.updateColors() || visible_scalar_field_ != name)
    {
        setVertexColorAttribute(sf.colors_);
        visible_scalar_field_ = name;
    }
}

void SurfaceMesh::hideAllScalarFields()
{
    if (visible_scalar_field_ != "(None)")
    {
        setVertexColorAttribute(color_);
        visible_scalar_field_ = "(None)";
    }
}

void SurfaceMesh::addVertexVectorField(std::string name, const VectorField &vf)
{
    vertex_vector_fields_[name] = vf;
}

void SurfaceMesh::removeVertexVectorField(std::string name)
{
    vertex_vector_fields_.erase(name);
}

void SurfaceMesh::addFaceVectorField(std::string name, const VectorField &vf)
{
    face_vector_fields_[name] = vf;
}

void SurfaceMesh::removeFaceVectorField(std::string name)
{
    face_vector_fields_.erase(name);
}

const VectorField &SurfaceMesh::vertexVectorField(std::string name) const
{
    return vertex_vector_fields_.at(name);
}

VectorField &SurfaceMesh::vertexVectorField(std::string name)
{
    return vertex_vector_fields_.at(name);
}

const VectorField &SurfaceMesh::faceVectorField(std::string name) const
{
    return face_vector_fields_.at(name);
}

VectorField &SurfaceMesh::faceVectorField(std::string name)
{
    return face_vector_fields_.at(name);
}

void SurfaceMesh::setFaceArrowMesh(const TriangleMeshData &mesh)
{
    
}

void SurfaceMesh::setVertexArrowMesh(const TriangleMeshData &mesh)
{
    size_t voffset = numVertices();
    size_t foffset = numFaces();
    glm::vec3 *pos = attributes_["positions"]->ptrVec3();
    glm::vec3 *nor = attributes_["normals"]->ptrVec3();
    glm::vec3 *col = attributes_["colors"]->ptrVec3();
    for (size_t i = 0; i < mesh.vertices.size(); ++i)
    {
        const size_t k = voffset + i;
        pos[k] = mesh.vertices.at(i);
        nor[k] = mesh.normals.at(i);
        col[k] = mesh.colors.at(i);
    }
    glm::uvec3 *ind = indices_->ptrUVec3();
    unsigned int offset(numVertices());
    for (size_t i = 0; i < mesh.indices.size(); ++i)
    {
        ind[foffset + i] = offset + mesh.indices.at(i);
    }
    uploadToGPU();
}

void SurfaceMesh::showVertexVectorField(std::string name)
{
    VectorField &vf = vertexVectorField(name);
    if (vf.updateArrows(vertices_) || visible_vertex_vector_field_ != name)
    {
        setVertexArrowMesh(vf.mesh_);
        visible_vertex_vector_field_ = name;
    }
}

void SurfaceMesh::showFaceVectorField(std::string name)
{
}

void SurfaceMesh::hideAllVertexVectorFields()
{
    if (visible_vertex_vector_field_ != "(None)")
    {
        size_t voffset = numVertices() * 3;
        size_t foffset = numFaces() * 3;
        std::fill(attributes_["positions"]->data().begin() + voffset,
                  attributes_["positions"]->data().end(), 0.f);
        std::fill(attributes_["normals"]->data().begin() + voffset,
                  attributes_["normals"]->data().end(), 0.f);
        std::fill(attributes_["colors"]->data().begin() + voffset,
                  attributes_["colors"]->data().end(), 0.f);
        std::fill(indices_->data().begin() + foffset, indices_->data().end(), 0);
        uploadToGPU();
        visible_vertex_vector_field_ = "(None)";
    }
}

void SurfaceMesh::hideAllFaceVectorFields()
{
}

glm::vec3 SurfaceMesh::color() const
{
    return color_;
}

void SurfaceMesh::setColor(const glm::vec3 &col)
{
    color_ = col;
    if (visible_scalar_field_ == "(None)")
    {
        setVertexColorAttribute(color_);
    }
}
void SurfaceMesh::drawGUI()
{
    Mesh::drawGUI();
    ImGui::Separator();
    ImGui::Text("Surface mesh geometry");
    if (ImGui::ColorEdit3("Color", glm::value_ptr(color_)))
    {
        setColor(color_);
    }

    // Scalar fields
    if (scalar_fields_.size() > 0)
    {
        ImGui::Separator();
    }
    static const char *current_sf = "(None)";
    if (ImGui::BeginCombo("Scalar field", current_sf))
    {
        bool is_selected = (current_sf == "(None)");
        if (ImGui::Selectable("(None)", is_selected))
        {
            current_sf = "(None)";
        }
        for (auto &kv : scalar_fields_)
        {
            bool is_selected = (current_sf == kv.first.c_str());
            if (ImGui::Selectable(kv.first.c_str(), is_selected))
            {
                current_sf = kv.first.c_str();
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    if (current_sf != nullptr && current_sf != "(None)")
    {
        showScalarField(current_sf);
        if (ImGui::InputFloat("Min. value", &scalarField(current_sf).vmin_))
        {
            scalarField(current_sf).dirty_ = true;
        }
        if (ImGui::InputFloat("Max. value", &scalarField(current_sf).vmax_))
        {
            scalarField(current_sf).dirty_ = true;
        }
        if (ImGui::Button("Fit data range"))
        {
            scalarField(current_sf).fitDataRange();
        }
    }
    if (current_sf == "(None)")
    {
        hideAllScalarFields();
    }
    // Vector fields
    if (vertex_vector_fields_.size() > 0)
    {
        ImGui::Separator();
    }
    static const char *current_vf = "(None)";
    if (ImGui::BeginCombo("Vertex vector field", current_vf))
    {
        bool is_selected = (current_vf == "(None)");
        if (ImGui::Selectable("(None)", is_selected))
        {
            current_vf = "(None)";
        }
        for (auto &kv : vertex_vector_fields_)
        {
            bool is_selected = (current_vf == kv.first.c_str());
            if (ImGui::Selectable(kv.first.c_str(), is_selected))
            {
                current_vf = kv.first.c_str();
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    if (current_vf != nullptr && current_vf != "(None)")
    {
        showVertexVectorField(current_vf);
        if (ImGui::SliderFloat("Max. length", &vertexVectorField(current_vf).max_length_, 0.f,
                               1.f))
        {
            vertexVectorField(current_vf).dirty_ = true;
        }
        if (ImGui::Checkbox("Scale by magnitude",
                            &vertexVectorField(current_vf).scale_by_magnitude_))
        {
            vertexVectorField(current_vf).dirty_ = true;
        }
    }
    if (current_vf == "(None)")
    {
        hideAllVertexVectorFields();
    }
}

void SurfaceMesh::updateBVH()
{
    // TODO(pradeep): find a way to avoid creating all these primitives and reuse original mesh data
    std::vector<PrimitivePtr> prims;
    const glm::vec3 *pos = attributes_["positions"]->ptrVec3();
    const unsigned int *ind = indices_->ptr();
    if (numIndexData() > 0)
    {
        prims.reserve(indices_->size() / 3);
        size_t face_id = 0;
        for (size_t i = 0; i < numFaces(); i += 3)
        {
            prims.push_back(std::make_shared<Triangle>(face_id++, pos[ind[i + 0]], pos[ind[i + 1]],
                                                       pos[ind[i + 2]]));
        }
    }
    else
    {
        size_t num_verts = numVertexData();
        prims.reserve(num_verts / 3);
        size_t face_id = 0;
        for (size_t i = 0; i < numVertices(); i += 3)
        {
            prims.push_back(
                std::make_shared<Triangle>(face_id++, pos[i + 0], pos[i + 1], pos[i + 2]));
        }
    }
    bvh_ = buildBVH(prims);
}

} // namespace viewer
} // namespace rcube