#include "RCubeViewer/RCubeViewer.h"

int main()
{
    using namespace rcube;
    using namespace rcube::viewer;

    // Properties to configure the viewer
    RCubeViewerProps props;
    props.resolution = glm::vec2(1280 /*4096*/, 720 /*2160*/);         // 720p
    props.MSAA = 2;                                                    // turn on 2x multisampling
    props.render_system = RCubeViewerProps::RenderSystemType::Forward; // Use forward rendering (this is the default)

    // Create a viewer
    viewer::RCubeViewer viewer(props);

    // Add a subdivided icosahedron surface to viewer
    // The returned entity has 2 components in it: (1) a Drawable component holding the mesh (2) a ForwardMaterial or 
    // DeferredMaterial component (based on the render system), (3) a Transform component holding the local position,
    // and local orientation with respect to a parent transform
    EntityHandle icoSphere = viewer.addSurface("icoSphere", rcube::icoSphere(1.0, 4));
    icoSphere.get<ForwardMaterial>()->shader = std::make_shared<MatCapRGBMaterial>();

    // To get the added sphere by name:
    // EntityHandle icoSphere = viewer.getEntity("icoSphere");
    // assert(icoSphere.valid());

    // Change its diffuse color by getting the Drawable component
    MatCapRGBMaterial *mat =
        std::static_pointer_cast<MatCapRGBMaterial>(icoSphere.get<ForwardMaterial>()->shader).get();
    mat->color = glm::vec3(0.0, 0.3, 0.7);
    mat->wireframe = true;

    // Show viewer
    viewer.execute();
    return 0;
}