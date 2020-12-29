#include "RCube/Core/Graphics/MeshGen/Points.h"
#include "RCubeViewer/Pointcloud.h"
#include "RCubeViewer/MessageBox.h"
#include "imgui.h"

namespace rcube
{
namespace viewer
{

void Pointcloud::createMesh()
{
    if (glyph_ == PointcloudGlyph::Sphere)
    {
        points_mesh_ = pointsToSpheres(points_, point_size_, num_vertices_per_point_,
                                       num_triangles_per_point_);
    }
    else
    {
        points_mesh_ =
            pointsToBoxes(points_, point_size_, num_vertices_per_point_, num_triangles_per_point_);
    }

    attributes_["positions"]->setData(points_mesh_.vertices);
    attributes_["normals"]->setData(points_mesh_.normals);
    attributes_["colors"]->setData(
        std::vector<glm::vec3>(points_mesh_.vertices.size(), glm::vec3(1, 1, 1)));
    indices_->setData(points_mesh_.indices);

    updateBVH();

    if (visible_scalar_field_ != "(None)")
    {
        scalarField(visible_scalar_field_).dirty_ = true;
    }
    if (visible_vector_field_ != "(None)")
    {
        vectorField(visible_vector_field_).dirty_ = true;
        setPointcloudArrowAttributes(vectorField(visible_vector_field_).mesh_);
    }
    uploadToGPU();
}

Pointcloud::Pointcloud(const std::vector<glm::vec3> &points, float point_size,
                       PointcloudGlyph glyph)
    : Mesh({AttributeBuffer::create("positions", GLuint(AttributeLocation::POSITION), 3),
            AttributeBuffer::create("normals", GLuint(AttributeLocation::NORMAL), 3),
            AttributeBuffer::create("colors", GLuint(AttributeLocation::COLOR), 3)},
           MeshPrimitive::Triangles, true),
      points_(points), point_size_(point_size), glyph_(glyph)
{
    createMesh();
}

std::shared_ptr<Pointcloud> Pointcloud::create(const std::vector<glm::vec3> &points,
                                               float point_size, PointcloudGlyph glyph)
{
    return std::shared_ptr<Pointcloud>(new Pointcloud(points, point_size, glyph));
}
size_t Pointcloud::verticesPerPoint() const
{
    return num_vertices_per_point_;
}
size_t Pointcloud::trianglesPerPoint() const
{
    return num_triangles_per_point_;
}
void Pointcloud::addScalarField(std::string name, const ScalarField &sf)
{
    assert(sf.data.size() == numPoints());
    scalar_fields_[name] = sf;
}
void Pointcloud::removeScalarField(std::string name)
{
    scalar_fields_.erase(name);
    visible_scalar_field_ = name;
}
const ScalarField &Pointcloud::scalarField(std::string name) const
{
    return scalar_fields_.at(name);
}
ScalarField &Pointcloud::scalarField(std::string name)
{
    return scalar_fields_.at(name);
}
void Pointcloud::showScalarField(std::string name)
{
    ScalarField &sf = scalarField(name);
    // Update the scalar field if it was updated or
    // the selected one is different from what's being displayed
    if (sf.updateColors() || visible_scalar_field_ != name)
    {
        setPointcloudColorAttribute(sf.colors_);
        visible_scalar_field_ = name;
    }
}
void Pointcloud::setPointcloudColorAttribute(const std::vector<glm::vec3> &perPointColors)
{
    assert(perPointColors.size() == numPoints());
    glm::vec3 *colors = attributes_["colors"]->ptrVec3();
    size_t k = 0;
    for (size_t i = 0; i < numPoints(); ++i)
    {
        for (size_t j = 0; j < verticesPerPoint(); ++j)
        {
            colors[k++] = perPointColors[i];
        }
    }
    uploadToGPU("colors");
}
void Pointcloud::setPointcloudColorAttribute(const glm::vec3 &perPointColor)
{
    glm::vec3 *colors = attributes_["colors"]->ptrVec3();
    size_t k = 0;
    for (size_t i = 0; i < numPoints(); ++i)
    {
        for (size_t j = 0; j < verticesPerPoint(); ++j)
        {
            colors[k++] = perPointColor;
        }
    }
    uploadToGPU("colors");
}
void Pointcloud::setPointcloudArrowAttributes(const TriangleMeshData &mesh)
{
    if (attributes_["positions"]->count() != numPoints() * verticesPerPoint() + mesh.vertices.size())
    {
        attributes_["positions"]->data().resize(numPoints() * verticesPerPoint() * 3 +
                                                mesh.vertices.size() * 3);
        attributes_["normals"]->data().resize(numPoints() * verticesPerPoint() * 3 +
                                              mesh.vertices.size() * 3);
        attributes_["colors"]->data().resize(numPoints() * verticesPerPoint() * 3 +
                                             mesh.vertices.size() * 3);
    }
    if (indices_->count() != numPoints() * trianglesPerPoint() + mesh.indices.size())
    {
        indices_->data().resize(numPoints() * trianglesPerPoint() * 3 + mesh.indices.size() * 3);
    }
    glm::vec3 *positions = attributes_["positions"]->ptrVec3();
    glm::vec3 *normals = attributes_["normals"]->ptrVec3();
    glm::vec3 *colors = attributes_["colors"]->ptrVec3();
    size_t k = numPoints() * num_vertices_per_point_;
    for (size_t i = 0; i < mesh.vertices.size(); ++i)
    {
        positions[k + i] = mesh.vertices.at(i);
        normals[k + i] = mesh.normals.at(i);
        colors[k + i] = mesh.colors.at(i);
    }
    glm::uvec3 *indices = indices_->ptrUVec3();
    k = numPoints() * num_triangles_per_point_;
    unsigned int offset(numPoints() * verticesPerPoint());
    for (size_t i = 0; i < mesh.indices.size(); ++i)
    {
        indices[k + i] = offset + mesh.indices[i];
    }
    uploadToGPU();
}
void Pointcloud::hideAllScalarFields()
{
    if (visible_scalar_field_ != "(None)")
    {
        setPointcloudColorAttribute(color_);
        visible_scalar_field_ = "(None)";
    }
}
void Pointcloud::updatePoints(const std::vector<glm::vec3> &points, float point_size)
{
    assert(points.size() == numPoints());
    bool ok = (points.size() == numPoints());
    if (!ok)
    {
        messageBoxError("Error", "Number of points passed in Pointcloud::updatePoints has to be " +
                                     std::to_string(numPoints()) + " but got " +
                                     std::to_string(points.size()));
        return;
    }
    TriangleMeshData trimesh =
        pointsToSpheres(points, point_size, num_vertices_per_point_, num_triangles_per_point_);
    attributes_["positions"]->setData(trimesh.vertices);
    attributes_["normals"]->setData(trimesh.normals);
    indices_->setData(trimesh.indices);
    uploadToGPU();
    updateBVH();
}
void Pointcloud::drawGUI()
{
    Mesh::drawGUI();
    ImGui::Separator();
    ImGui::Text("Pointcloud geometry");
    ImGui::LabelText("#points", std::to_string(numPoints()).c_str());
    if (ImGui::InputFloat("Point size", &point_size_))
    {
        point_size_ = std::max(0.0001f, point_size_);
        createMesh();
    }
    if (ImGui::ColorEdit3("Color", glm::value_ptr(color_)))
    {
        setColor(color_);
    }
    ImGui::Text("Glyph");
    if (ImGui::RadioButton("Sphere", glyph_ == PointcloudGlyph::Sphere))
    {
        glyph_ = PointcloudGlyph::Sphere;
        createMesh();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Box", glyph_ == PointcloudGlyph::Box))
    {
        glyph_ = PointcloudGlyph::Box;
        createMesh();
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
    if (vector_fields_.size() > 0)
    {
        ImGui::Separator();
    }
    static const char *current_vf = "(None)";
    if (ImGui::BeginCombo("Vector field", current_vf))
    {
        bool is_selected = (current_vf == "(None)");
        if (ImGui::Selectable("(None)", is_selected))
        {
            current_vf = "(None)";
        }
        for (auto &kv : vector_fields_)
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
        showVectorField(current_vf);
        if (ImGui::SliderFloat("Max. length", &vectorField(current_vf).max_length_, 0.f,
                               10.f * point_size_))
        {
            vectorField(current_vf).dirty_ = true;
        }
        if (ImGui::Checkbox("Scale by magnitude", &vectorField(current_vf).scale_by_magnitude_))
        {
            vectorField(current_vf).dirty_ = true;
        }
    }
    if (current_vf == "(None)")
    {
        hideAllVectorFields();
    }
}
size_t Pointcloud::numPoints() const
{
    return points_.size();
}
float Pointcloud::pointSize() const
{
    return point_size_;
}
glm::vec3 Pointcloud::color() const
{
    return color_;
}
void Pointcloud::setColor(const glm::vec3 &col)
{
    color_ = col;
    if (visible_scalar_field_ == "(None)")
    {
        setPointcloudColorAttribute(color_);
    }
}
void Pointcloud::addVectorField(std::string name, const VectorField &vf)
{
    assert(vf.data.size() == numPoints());
    vector_fields_[name] = vf;
}
void Pointcloud::removeVectorField(std::string name)
{
    vector_fields_.erase(name);
    visible_vector_field_ = "(None)";
}
const VectorField &Pointcloud::vectorField(std::string name) const
{
    return vector_fields_.at(name);
}
VectorField &Pointcloud::vectorField(std::string name)
{
    return vector_fields_.at(name);
}
void Pointcloud::showVectorField(std::string name)
{
    VectorField &vf = vectorField(name);
    bool update = vf.updateArrows(points_);
    if (update || visible_vector_field_ != name)
    {
        setPointcloudArrowAttributes(vf.mesh_);
        visible_vector_field_ = name;
    }
}
void rcube::viewer::Pointcloud::hideAllVectorFields()
{
    if (visible_vector_field_ != "(None)")
    {
        attributes_["positions"]->data().resize(numPoints() * num_vertices_per_point_ * 3);
        attributes_["normals"]->data().resize(numPoints() * num_vertices_per_point_ * 3);
        attributes_["colors"]->data().resize(numPoints() * num_vertices_per_point_ * 3);
        indices_->data().resize(numPoints() * num_triangles_per_point_ * 3);
        uploadToGPU();
        visible_vector_field_ = "(None)";
    }
}
} // namespace viewer
} // namespace rcube
