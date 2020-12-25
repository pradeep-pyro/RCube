#include "RCubeViewer/Pointcloud.h"
#include "RCube/Core/Graphics/MeshGen/Points.h"
#include "RCubeViewer/Colormap.h"
#include "RCubeViewer/Components/Name.h"
#include "RCubeViewer/Components/Pickable.h"
#include "RCubeViewer/RCubeViewer.h"
#include "imgui.h"
#include <random>

int main()
{
    using namespace rcube;
    using namespace rcube::viewer;

    // Properties to configure the viewer
    RCubeViewerProps props;
    props.resolution = glm::vec2(1280 /*4096*/, 720 /*2160*/); // 720p
    props.MSAA = 2;                                            // turn on 2x multisampling
    props.camera_fov = glm::radians(30.f);

    // Create a viewer
    RCubeViewer viewer(props);

    // Create a pointcloud with the vertices of an icosphere
    auto sphere = icoSphere(0.5f, 4);

    // Convert points to mesh for visualization
    std::shared_ptr<Pointcloud> pc = Pointcloud::create(sphere.vertices, 0.01f);
    ScalarField xs, ys, zs;
    for (const glm::vec3 &xyz : sphere.vertices)
    {
        xs.data().push_back(xyz.x);
        ys.data().push_back(xyz.y);
        zs.data().push_back(xyz.z);
    }
    pc->addScalarField("Xs", xs);
    pc->addScalarField("Ys", ys);
    pc->addScalarField("Zs", zs);

    // Add an entity in the viewer to hold the pointcloud
    auto entity = viewer.createPointcloudEntity("Sphere vertices");
    entity.get<Drawable>()->mesh = pc;

    // Make pointcloud pickable with mouse click
    entity.add<Pickable>();

    // Custom window to show picked point
    viewer.customGUI = [&](RCubeViewer &v) {
        ImGui::Begin("Pick");
        Pickable *pick_comp = entity.get<Pickable>();
        if (pick_comp->picked)
        {
            size_t index = pick_comp->primitive;
            ImGui::Text("Picked point: %zd\n", index);
            // Get the pointcloud
            std::shared_ptr<Pointcloud> pc =
                std::dynamic_pointer_cast<Pointcloud>(entity.get<Drawable>()->mesh);

            // Display the scalar fields values for the picked point
            ImGui::LabelText("Xs", std::to_string(pc->scalarField("Xs").data()[index]).c_str());
            ImGui::LabelText("Ys", std::to_string(pc->scalarField("Ys").data()[index]).c_str());
            ImGui::LabelText("Zs", std::to_string(pc->scalarField("Zs").data()[index]).c_str());
        }
        ImGui::End();
    };

    // Show viewer
    viewer.execute();
    return 0;
}