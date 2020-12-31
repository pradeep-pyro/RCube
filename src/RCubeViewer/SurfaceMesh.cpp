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
    faces_ = data.indices;
    face_centers_.clear();
    face_centers_.reserve(data.indices.size());
    for (const glm::uvec3 &f : data.indices)
    {
        face_centers_.push_back((vertices_[f[0]] + vertices_[f[1]] + vertices_[f[2]]) / 3.f);
    }
    // These are the vertices actually used for displaying the mesh
    // Each vertex in a face is duplicated to support per-face scalar
    // fields.
    vertices_display_.clear();
    vertices_display_.reserve(data.indices.size() * 3);
    for (const glm::uvec3 &f : data.indices)
    {
        vertices_display_.push_back(vertices_[f[0]]);
        vertices_display_.push_back(vertices_[f[1]]);
        vertices_display_.push_back(vertices_[f[2]]);
    }
    // The mesh attributes are setup such that the vertices of the surface come first,
    // followed by the vertices of the vertex vector field, followed by the vertices
    // of the face vector field
    // First, find the number of vertices needed for the arrow meshes
    size_t num_arrow_vertices;
    TriangleMeshData dummy_arrow;
    pointsVectorsToArrows({glm::vec3(0, 0, 0)}, {glm::vec3(0, 1, 0)}, {1.f}, num_arrow_vertices,
                          dummy_arrow);
    // Total number of vertices for the vertex vector field is num_arrow_vertices multiplied
    // by number of vertices
    num_vertex_vector_field_vertices_ = num_arrow_vertices * numVertices();
    // Similarly, number of vertices for face vector field is num_arrow_vertices multiplied
    // by number of faces
    num_face_vector_field_vertices_ = num_arrow_vertices * numFaces();

    attributes_["positions"]->data().resize(3 * (numVerticesDisplay() +
                                                 num_vertex_vector_field_vertices_ +
                                                 num_face_vector_field_vertices_),
                                            0.f);
    attributes_["normals"]->data().resize(3 * (numVerticesDisplay() +
                                               num_vertex_vector_field_vertices_ +
                                               num_face_vector_field_vertices_),
                                          0.f);
    attributes_["colors"]->data().resize(3 * (numVerticesDisplay() +
                                              num_vertex_vector_field_vertices_ +
                                              num_face_vector_field_vertices_),
                                         0.f);
    glm::vec3 *pos = attributes_["positions"]->ptrVec3();
    glm::vec3 *nor = attributes_["normals"]->ptrVec3();
    glm::vec3 *col = attributes_["colors"]->ptrVec3();
    size_t k = 0;
    for (const glm::uvec3 &ind : data.indices)
    {
        for (size_t j = 0; j < 3; ++j)
        {
            pos[k] = data.vertices.at(size_t(ind[j]));
            nor[k] = data.normals.at(size_t(ind[j]));
            col[k] = color_;
            ++k;
        }
    }
    std::vector<PrimitivePtr> prims;
    prims.reserve(data.indices.size());
    for (size_t i = 0; i < data.indices.size(); ++i)
    {
        const glm::uvec3 ind = data.indices[i];
        prims.push_back(std::make_shared<Triangle>(i, data.vertices[ind[0]], data.vertices[ind[1]],
                                                   data.vertices[ind[2]]));
    }
    updateBVH(prims);
    uploadToGPU();
}

SurfaceMesh::SurfaceMesh(const TriangleMeshData &data)
    : Mesh({AttributeBuffer::create("positions", GLuint(AttributeLocation::POSITION), 3),
            AttributeBuffer::create("normals", GLuint(AttributeLocation::NORMAL), 3),
            AttributeBuffer::create("colors", GLuint(AttributeLocation::COLOR), 3)},
           MeshPrimitive::Triangles, false)
{
    createMesh(data);
}

std::shared_ptr<SurfaceMesh> SurfaceMesh::create(const TriangleMeshData &data)
{
    return std::shared_ptr<SurfaceMesh>(new SurfaceMesh(data));
}

void SurfaceMesh::setVertexColorAttribute(const std::vector<glm::vec3> &vertex_colors)
{
    assert(vertex_colors.size() == numVertices());
    glm::vec3 *colors = attributes_["colors"]->ptrVec3();
    size_t k = 0;
    for (const glm::uvec3 &ind : faces_)
    {
        colors[k++] = vertex_colors[ind[0]];
        colors[k++] = vertex_colors[ind[1]];
        colors[k++] = vertex_colors[ind[2]];
    }
    uploadToGPU("colors");
}

void SurfaceMesh::setVertexColorAttribute(const glm::vec3 &vertex_color)
{
    glm::vec3 *colors = attributes_["colors"]->ptrVec3();
    size_t k = 0;
    for (size_t i = 0; i < faces_.size(); ++i)
    {
        colors[k++] = vertex_color;
        colors[k++] = vertex_color;
        colors[k++] = vertex_color;
    }
    uploadToGPU("colors");
}

void SurfaceMesh::setFaceColorAttribute(const std::vector<glm::vec3> &per_face_colors)
{
    assert(per_face_colors.size() == numFaces());
    glm::vec3 *colors = attributes_["colors"]->ptrVec3();
    size_t k = 0;
    for (const glm::vec3 &face_color : per_face_colors)
    {
        colors[k++] = face_color;
        colors[k++] = face_color;
        colors[k++] = face_color;
    }
    uploadToGPU("colors");
}

void SurfaceMesh::setFaceColorAttribute(const glm::vec3 &face_color)
{
    setVertexColorAttribute(face_color);
}

size_t SurfaceMesh::numVertices() const
{
    return vertices_.size();
}

size_t SurfaceMesh::numFaces() const
{
    return face_centers_.size();
}

void SurfaceMesh::addVertexScalarField(std::string name, const ScalarField &sf)
{
    vertex_scalar_fields_[name] = sf;
}

void SurfaceMesh::addFaceScalarField(std::string name, const ScalarField &sf)
{
    face_scalar_fields_[name] = sf;
}

void SurfaceMesh::removeFaceScalarField(std::string name)
{
    face_scalar_fields_.erase(name);
}

const ScalarField &SurfaceMesh::faceScalarField(std::string name) const
{
    return face_scalar_fields_.at(name);
}

ScalarField &SurfaceMesh::faceScalarField(std::string name)
{
    return face_scalar_fields_.at(name);
}

void SurfaceMesh::showFaceScalarField(std::string name)
{
    ScalarField &sf = faceScalarField(name);
    // Update the scalar field if it was updated or
    // the selected one is different from what's being displayed
    if (sf.updateColors() || visible_face_scalar_field_ != name)
    {
        setFaceColorAttribute(sf.colors_);
        visible_face_scalar_field_ = name;
        visible_vertex_scalar_field_ = "(None)";
    }
}

void SurfaceMesh::removeVertexScalarField(std::string name)
{
    vertex_scalar_fields_.erase(name);
}

const ScalarField &SurfaceMesh::vertexScalarField(std::string name) const
{
    return vertex_scalar_fields_.at(name);
}

ScalarField &SurfaceMesh::vertexScalarField(std::string name)
{
    return vertex_scalar_fields_.at(name);
}

void SurfaceMesh::showVertexScalarField(std::string name)
{
    ScalarField &sf = vertexScalarField(name);
    // Update the scalar field if it was updated or
    // the selected one is different from what's being displayed
    if (sf.updateColors() || visible_vertex_scalar_field_ != name)
    {
        setVertexColorAttribute(sf.colors_);
        visible_vertex_scalar_field_ = name;
        visible_face_scalar_field_ = "(None)";
    }
}

void SurfaceMesh::hideAllScalarFields()
{
    if (visible_vertex_scalar_field_ == "(None)" && visible_face_scalar_field_ == "(None)")
    {
        return;
    }
    setVertexColorAttribute(color_);
    visible_vertex_scalar_field_ = "(None)";
    visible_face_scalar_field_ = "(None)";
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

size_t SurfaceMesh::numVerticesDisplay() const
{
    return face_centers_.size() * 3;
}

void SurfaceMesh::setVertexArrowMesh(const TriangleMeshData &mesh)
{
    size_t voffset = numVerticesDisplay();
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
    VectorField &vf = vertexVectorField(name);
    if (vf.updateArrows(face_centers_) || visible_face_vector_field_ != name)
    {
        setVertexArrowMesh(vf.mesh_);
        visible_face_vector_field_ = name;
    }
}

void SurfaceMesh::hideAllVertexVectorFields()
{
    if (visible_vertex_vector_field_ != "(None)")
    {
        size_t voffset = numVerticesDisplay() * 3;
        std::fill(attributes_["positions"]->data().begin() + voffset,
                  attributes_["positions"]->data().end(), 0.f);
        std::fill(attributes_["normals"]->data().begin() + voffset,
                  attributes_["normals"]->data().end(), 0.f);
        std::fill(attributes_["colors"]->data().begin() + voffset,
                  attributes_["colors"]->data().end(), 0.f);
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
    if (visible_vertex_scalar_field_ == "(None)" && visible_face_scalar_field_ == "(None)")
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
    /////////////////////////////////////////////////////////
    // Scalar fields
    // -> Vertex
    ImGui::Separator();
    static const char *current_sf = nullptr;
    static bool is_vertex_based = true;
    if (ImGui::BeginCombo("Scalar field", current_sf))
    {
        bool is_selected = (current_sf == "(None)");
        if (ImGui::Selectable("(None)", is_selected))
        {
            current_sf = "(None)";
        }
        bool temp;
        ImGui::Selectable("Per-vertex", temp, ImGuiSelectableFlags_Disabled);
        for (auto &kv : vertex_scalar_fields_)
        {
            bool is_selected = (current_sf == kv.first.c_str());
            if (ImGui::Selectable(kv.first.c_str(), is_selected))
            {
                current_sf = kv.first.c_str();
                is_vertex_based = true;
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::Separator();
        ImGui::Selectable("Per-face", temp, ImGuiSelectableFlags_Disabled);
        for (auto &kv : face_scalar_fields_)
        {
            bool is_selected = (current_sf == kv.first.c_str());
            if (ImGui::Selectable(kv.first.c_str(), is_selected))
            {
                current_sf = kv.first.c_str();
                is_vertex_based = false;
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
        if (is_vertex_based)
        {
            showVertexScalarField(current_sf);
            if (ImGui::InputFloat("Min. value###sf1", &vertexScalarField(current_sf).vmin_))
            {
                vertexScalarField(current_sf).dirty_ = true;
            }
            if (ImGui::InputFloat("Max. value###sf2", &vertexScalarField(current_sf).vmax_))
            {
                vertexScalarField(current_sf).dirty_ = true;
            }
            if (ImGui::Button("Fit data range###sf3"))
            {
                vertexScalarField(current_sf).fitDataRange();
            }
        }
        else
        {
            showFaceScalarField(current_sf);
            if (ImGui::InputFloat("Min. value###sf4", &faceScalarField(current_sf).vmin_))
            {
                faceScalarField(current_sf).dirty_ = true;
            }
            if (ImGui::InputFloat("Max. value###sf5", &faceScalarField(current_sf).vmax_))
            {
                faceScalarField(current_sf).dirty_ = true;
            }
            if (ImGui::Button("Fit data range###sf6"))
            {
                faceScalarField(current_sf).fitDataRange();
            }
        }
    }
    if (current_sf == "(None)")
    {
        hideAllScalarFields();
    }
    /////////////////////////////////////////////////////////
    // Vector fields
    // -> Vertex
    ImGui::Separator();
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
        if (ImGui::SliderFloat("Max. length", &vertexVectorField(current_vf).max_length_, 0.f, 1.f))
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

} // namespace viewer
} // namespace rcube