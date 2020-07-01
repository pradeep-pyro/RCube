#include "RCube/Core/Graphics/Effects/GammaCorrectionEffect.h"
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

    // Properties to configure the viewer
    viewer::RCubeViewerProps props;
    props.resolution = glm::vec2(1280 /*4096*/, 720 /*2160*/); // 720p
    props.MSAA = 2;                                            // turn on 2x multisampling
    props.camera_fov = glm::radians(30.f);

    // Create a viewer
    viewer::RCubeViewer viewer(props);

    // Create a random pointcloud
    size_t num_points = 1024;
    std::vector<glm::vec3> pointcloud(num_points);
    std::uniform_real_distribution<float> dist(-1.f, 1.f);
    std::default_random_engine gen;
    for (size_t i = 0; i < num_points; ++i)
    {
        pointcloud[i] = glm::vec3(dist(gen), dist(gen), dist(gen));
    }

    // Convert points to mesh for visualization
    size_t num_triangles_per_point;
    TriangleMeshData pointcloudMesh = pointsToSpheres(pointcloud, 0.03f, num_triangles_per_point);
    EntityHandle pc_entity = viewer.addSurface("pointcloud", pointcloudMesh);
    auto mat =
        std::dynamic_pointer_cast<PhysicallyBasedMaterial>(pc_entity.get<Drawable>()->material);
    mat->albedo = glm::vec3(0.8, 0.8, 0.8);
    mat->roughness = 0.8;
    mat->metallic = 0.3;

    // Make pointcloud pickable with mouse click
    pc_entity.add<viewer::Pickable>();
    pc_entity.get<Drawable>()->mesh->updateBVH();

    viewer.customGUI = [&](viewer::RCubeViewer &v) {
        ImGui::Begin("Pick");
        viewer::Pickable *pick_comp = pc_entity.get<viewer::Pickable>();
        if (pick_comp->picked)
        {
            ImGui::Text(
                "Picked triangle index %zd, which corresponds to pointcloud index %zd\n",
                pick_comp->triangle,
                (size_t)std::floor((double)pick_comp->triangle / (double)num_triangles_per_point));
        }
        ImGui::End();
    };

    // Apply gamma correction to the screen
    viewer.camera().get<Camera>()->postprocess.push_back(makeGammaCorrectionEffect());

    // Compute IBL to all PBR materials
    viewer.updateImageBasedLighting();

    // Show viewer
    viewer.execute();
    return 0;
}