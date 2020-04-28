#include "RCube/Core/Graphics/Effects/GammaCorrectionEffect.h"
#include "RCube/Core/Graphics/MeshGen/Points.h"
#include "RCubeViewer/Colormap.h"
#include "RCubeViewer/Components/Name.h"
#include "RCubeViewer/RCubeViewer.h"
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
    pc_entity.get<Drawable>()->material->uniform("diffuse").set(glm::vec3(0.8, 0.2, 0.0));

    // Make pointcloud pickable with mouse click
    pc_entity.get<Drawable>()->mesh->updateBVH();

    viewer.handleMouseDown = [&](rcube::viewer::RCubeViewer &v) -> bool {
        glm::dvec2 mouse_pos = v.getMousePosition();
        EntityHandle ent;
        size_t triangle_index;
        if (v.pick(int(mouse_pos.x), int(mouse_pos.y), ent, triangle_index))
        {
            std::cout << "Picked entity '" << ent.get<Name>()->name << "' by selecting triangle "
                      << triangle_index << ", which corresponds to pointcloud index "
                      << (size_t)std::floor((double)triangle_index /
                                            (double)num_triangles_per_point)
                      << std::endl;
        }
        return false; // Return false to process default mouse down stuff in the viewer
    };

    // Show viewer
    viewer.execute();
    return 0;
}