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
    std::shared_ptr<Pointcloud> pc = Pointcloud::create(sphere.vertices, 0.03f);
    
    // Add some scalar fields
    ScalarField xs, ys, zs;
    std::vector<float> xs_data, ys_data, zs_data;
    for (const glm::vec3 &xyz : sphere.vertices)
    {
        xs_data.push_back(xyz.x);
        ys_data.push_back(xyz.y);
        zs_data.push_back(xyz.z);
    }
    xs.setData(xs_data);
    ys.setData(ys_data);
    zs.setData(zs_data);
    pc->addScalarField("Xs", xs);
    pc->addScalarField("Ys", ys);
    pc->addScalarField("Zs", zs);

    // Add a vector field
    VectorField normals;
    normals.setVectors(sphere.normals);
    pc->addVectorField("Normals", normals);

    // Add an entity in the viewer to hold the pointcloud
    auto entity = viewer.addMeshEntity("Sphere vertices");
    entity.get<Drawable>()->mesh = pc;

    // Make pointcloud pickable with mouse
    entity.add<Pickable>();

    // Show viewer
    viewer.execute();
    return 0;
}