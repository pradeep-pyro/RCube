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
    // The returned entity has 2 components in it: (1) a Drawable component holding the mesh and
    // material (a physically-based material is used by default), (2) a Transform component holding
    // the local position, and local orientation with respect to a parent transform
    EntityHandle icoSphere = viewer.addSurface("icoSphere", rcube::icoSphere(1.0, 4));

    // To get the added sphere by name:
    // EntityHandle icoSphere = viewer.getEntity("icoSphere");
    // assert(icoSphere.valid());

    // Change its diffuse color by getting the Drawable component
    Material *material = icoSphere.get<Material>();
    material->albedo = glm::vec3(0.0, 0.3, 0.7);
    material->wireframe = true;

    // Show viewer
    viewer.execute();
    return 0;
}