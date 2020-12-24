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
    size_t num_vertices_per_point_ = 0;
    size_t num_triangles_per_point_ = 0;
    size_t num_points_ = 0;
    std::unordered_map<std::string, ScalarField> scalar_fields_;
    std::string visible_scalar_field_ = "";

    Pointcloud(const std::vector<glm::vec3> &points, float point_size)
        : Mesh({AttributeBuffer::create("positions", GLuint(AttributeLocation::POSITION), 3),
                AttributeBuffer::create("normals", GLuint(AttributeLocation::NORMAL), 3),
                AttributeBuffer::create("colors", GLuint(AttributeLocation::COLOR), 3)},
               MeshPrimitive::Triangles, true)
    {
        TriangleMeshData trimesh =
            pointsToSpheres(points, point_size, num_vertices_per_point_, num_triangles_per_point_);
        attributes_["positions"]->setData(trimesh.vertices);
        attributes_["normals"]->setData(trimesh.normals);
        indices_->setData(trimesh.indices);
        uploadToGPU();
        updateBVH();
        num_points_ = points.size();
    }

  public:
    static std::shared_ptr<Pointcloud> create(const std::vector<glm::vec3> &points,
                                              float point_size)
    {
        return std::shared_ptr<Pointcloud>(new Pointcloud(points, point_size));
    }
    size_t verticesPerPoint() const
    {
        return num_vertices_per_point_;
    }
    size_t trianglesPerPoint() const
    {
        return num_triangles_per_point_;
    }
    void addScalarField(std::string name, const ScalarField &sf)
    {
        assert(sf.data.size() == numPoints());
        scalar_fields_[name] = sf;
    }
    const ScalarField &scalarField(std::string name) const
    {
        return scalar_fields_.at(name);
    }
    ScalarField &scalarField(std::string name)
    {
        return scalar_fields_.at(name);
    }
    void showScalarField(std::string name)
    {
        ScalarField& sf = scalarField(name);
        if (visible_scalar_field_ != name || sf.dirty_)
        {
            sf.updateColors();
            std::vector<glm::vec3> colors;
            colors.reserve(sf.colors_.size() * verticesPerPoint());
            for (size_t i = 0; i < sf.colors_.size(); ++i)
            {
                for (size_t j = 0; j < verticesPerPoint(); ++j)
                {
                    colors.push_back(sf.colors_[i]);
                }
            }
            attributes_["colors"]->setData(colors);
            visible_scalar_field_ = name;
            uploadToGPU();
        }
    }
    void hideAllScalarFields()
    {
        attributes_["colors"]->setData(std::vector<glm::vec3>{});
        visible_scalar_field_ = "";
        uploadToGPU();
    }
    void updatePoints(const std::vector<glm::vec3> &points)
    {
        assert(points.size() == num_points_);
        attributes_["positions"]->setData(points);
        uploadToGPU();
        updateBVH();
    }
    virtual void drawGUI() override
    {
        ImGui::Separator();
        ImGui::Text("Pointcloud geometry");
        ImGui::LabelText("#points", std::to_string(num_points_).c_str());
        if (scalar_fields_.size() > 0)
        {
            ImGui::Separator();
        }
        static const char *current_sf = "(None)";
        if (ImGui::BeginCombo("Scalar field", current_sf))
        {
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
            if (ImGui::InputFloat("Min.", &scalarField(current_sf).vmin_))
            {
                scalarField(current_sf).dirty_ = true;
            }
            if (ImGui::InputFloat("Max.", &scalarField(current_sf).vmax_))
            {
                scalarField(current_sf).dirty_ = true;
            }
            if (ImGui::Button("Fit data range"))
            {
                scalarField(current_sf).fitDataRange();
            }
        }
    }
};

} // namespace viewer
} // namespace rcube