#include "RCube/Core/Graphics/Effects/GrayscaleEffect.h"
#include "RCube/Core/Graphics/Effects/GammaCorrectionEffect.h"
#include "RCube/Core/Graphics/Materials/PhysicallyBasedMaterial.h"
#include "RCubeViewer/RCubeViewer.h"

int main()
{
    using namespace rcube;

    viewer::RCubeViewer viewer;

    // Add a subdivided icosahedron surface to viewer
    viewer.addIcoSphereSurface("icoSphere1", 1.0, 4);

    // Get the added sphere by name
    EntityHandle icoSphere1 = viewer.getEntity("icoSphere1");
    assert(icoSphere1.valid());

    // Change its diffuse color
    const auto& material = icoSphere1.get<Drawable>()->material;
    material->uniform("material.diffuse").set(glm::vec3(0.0, 0.3, 0.7));

    // Apply grayscale filter to screen
    // viewer.camera().get<Camera>()->postprocess.push_back(makeGrayscaleEffect());

    // Apply gamma correction filter to screen
    viewer.camera().get<Camera>()->postprocess.push_back(makeGammaCorrectionEffect());

    // Show viewer
    viewer.execute();
    return 0;
}