#include "RCube/Core/Graphics/Effects/GammaCorrectionEffect.h"
#include "RCubeViewer/Colormap.h"
#include "RCubeViewer/RCubeViewer.h"

int main()
{
    using namespace rcube;

    // Properties to configure the viewer
    viewer::RCubeViewerProps props;
    props.resolution = glm::vec2(1280 /*4096*/, 720 /*2160*/); // 720p
    props.MSAA = 2;                                            // turn on 2x multisampling

    // Create a viewer
    viewer::RCubeViewer viewer(props);

    // Add a subdivided icosahedron surface to viewer
    TriangleMeshData icosphere = icoSphere(1.0, 4);
    EntityHandle sphere = viewer.addSurface("sphere", icosphere);

    // Create a scalar field
    std::vector<float> height_field;
    Mesh *sphere_mesh = sphere.get<Drawable>()->mesh.get();
    height_field.reserve(icosphere.vertices.size());
    for (auto &v : icosphere.vertices)
    {
        height_field.push_back(v.y);
    }

    // Create colors based on colormap
    float vmin = -1.f;
    float vmax = +1.f;
    colormap(Colormap::Viridis, height_field, vmin, vmax, sphere_mesh->attribute("colors")->data());
    sphere_mesh->uploadToGPU();

    // Set albedo color to white since it will be multiplied with the colors above
    auto material =
        std::dynamic_pointer_cast<PhysicallyBasedMaterial>(sphere.get<Drawable>()->material);

    // Show viewer
    viewer.execute();
    return 0;
}