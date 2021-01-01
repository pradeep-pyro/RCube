#include "RCubeViewer/Systems/PickTooltipSystem.h"
#include "RCube/Components/Drawable.h"
#include "RCube/Components/Transform.h"
#include "RCubeViewer/Components/Pickable.h"
#include "RCubeViewer/Pointcloud.h"
#include "RCubeViewer/SurfaceMesh.h"
#include "imgui.h"

namespace rcube
{
namespace viewer
{
PickTooltipSystem::PickTooltipSystem()
{
    addFilter({Drawable::family(), Transform::family(), Pickable::family()});
}

void vec3LabelText(const std::string &name, const glm::vec3 &vec)
{
    ImGui::LabelText(name.c_str(), (std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " +
                                    std::to_string(vec.z))
                                       .c_str());
}

void PickTooltipSystem::update(bool)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }
    for (Entity ent :
         getFilteredEntities({Drawable::family(), Transform::family(), Pickable::family()}))
    {
        Drawable *dr = world_->getComponent<Drawable>(ent);
        if (!dr->visible)
        {
            continue;
        }
        Pickable *pick = world_->getComponent<Pickable>(ent);
        if (pick->active && pick->picked)
        {
            ///////////////////////////////
            // Pointcloud
            {
                Pointcloud *pc = dynamic_cast<Pointcloud *>(dr->mesh.get());
                if (pc != nullptr)
                {
                    ImGui::BeginTooltip();
                    size_t index = pick->primitive->id();
                    ImGui::LabelText("Point", std::to_string(index).c_str());
                    const glm::vec3 &coord = pc->points_[index];
                    vec3LabelText("Coord", coord);
                    // Display the fields values for the picked point
                    if (pc->visible_scalar_field_ != "(None)")
                    {
                        ImGui::Separator();
                        const std::string name = pc->visible_scalar_field_;
                        ImGui::LabelText(
                            name.c_str(),
                            std::to_string(pc->scalarField(name).data()[index]).c_str());
                    }
                    if (pc->visible_vector_field_ != "(None)")
                    {
                        ImGui::Separator();
                        const std::string name = pc->visible_vector_field_;
                        const glm::vec3 vec = pc->vectorField(name).vectors()[index];
                        vec3LabelText(name, vec);
                    }
                    ImGui::EndTooltip();
                }
            }
            ///////////////////////////////
            // SurfaceMesh
            {
                SurfaceMesh *sm = dynamic_cast<SurfaceMesh *>(dr->mesh.get());
                if (sm != nullptr)
                {
                    ImGui::BeginTooltip();
                    size_t index = pick->primitive->id();
                    Triangle *tri = dynamic_cast<Triangle *>(pick->primitive.get());

                    float dist = 1e6f;
                    // TODO(pradeep): can avoid closest vertex check
                    size_t vijk = tri->closestVertexIndex(pick->point, dist);
                    if (tri->barycentricCoordinate(pick->point)[vijk] > 0.85f)
                    {
                        // Vertex was selected
                        size_t vertex_ind = sm->faces_[index][vijk];
                        ImGui::LabelText("Vertex", std::to_string(vertex_ind).c_str());
                        vec3LabelText("Coord", sm->vertices_[vertex_ind]);
                        if (sm->visible_vertex_scalar_field_ != "(None)")
                        {
                            ImGui::Separator();
                            const std::string name = sm->visible_vertex_scalar_field_;
                            float val = sm->vertexScalarField(name).data()[index];
                            ImGui::LabelText(name.c_str(), std::to_string(val).c_str());
                        }
                        if (sm->visible_vertex_vector_field_ != "(None)")
                        {
                            ImGui::Separator();
                            const std::string name = sm->visible_vertex_vector_field_;
                            const glm::vec3 &vec = sm->vertexVectorField(name).vectors()[index];
                            vec3LabelText(name, vec);
                        }
                    }
                    else
                    {
                        // Triangle was selected
                        ImGui::LabelText("Triangle", std::to_string(index).c_str());
                        if (sm->visible_face_scalar_field_ != "(None)")
                        {
                            ImGui::Separator();
                            const std::string name = sm->visible_face_scalar_field_;
                            float val = sm->faceScalarField(name).data()[index];
                            ImGui::LabelText(name.c_str(), std::to_string(val).c_str());
                        }
                        // TODO(pradeep): add face vector fields when ready
                    }
                    ImGui::EndTooltip();
                }
            }
        }
    }
}
unsigned int PickTooltipSystem::priority() const
{
    return 1201;
}
const std::string PickTooltipSystem::name() const
{
    return "PickTooltipSystem";
}
} // namespace viewer
} // namespace rcube