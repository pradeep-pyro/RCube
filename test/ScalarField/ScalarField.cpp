#include "RCube/Core/Graphics/Effects/GammaCorrectionEffect.h"
#include "RCubeViewer/RCubeViewer.h"
#include "RCubeViewer/Colormap.h"

int main()
{
    using namespace rcube;

    // Properties to configure the viewer
    viewer::RCubeViewerProps props;
    props.resolution = glm::vec2(1280 /*4096*/, 720 /*2160*/); // 720p
    // props.MSAA = 2;                                            // turn on 2x multisampling

    // Create a viewer
    viewer::RCubeViewer viewer(props);

    // Add a subdivided icosahedron surface to viewer
    EntityHandle sphere = viewer.addIcoSphereSurface("sphere", 1.0, 4);

    // Create a scalar field
    std::vector<float> height_field;
    Mesh *sphere_mesh = sphere.get<Drawable>()->mesh.get();
    height_field.reserve(sphere_mesh->data.vertices.size());
    for (auto &v : sphere_mesh->data.vertices)
    {
        height_field.push_back(v.y);
    }

    // Create colors based on colormap
    float vmin = -1.f;
    float vmax = +1.f;
    colormap(Colormap::Viridis, height_field, vmin, vmax, sphere_mesh->data.colors);
    sphere_mesh->uploadToGPU();
    // Set diffuse color to white since it will be multiplied with the colors above
    sphere.get<Drawable>()->material->uniform("diffuse").set(glm::vec3(1, 1, 1));

    // Show viewer
    viewer.execute();
    return 0;
}