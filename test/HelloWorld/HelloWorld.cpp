#include "RCubeViewer/RCubeViewer.h"

int main()
{
    rcube::viewer::RCubeViewer viewer;
    viewer.addIcoSphereSurface("icoSphere1", 1.0, 4);
    rcube::EntityHandle icoSphere1 = viewer.getEntity("icoSphere1");
    assert(icoSphere1.valid());
    {
        auto phong = std::dynamic_pointer_cast<rcube::BlinnPhongMaterial>(
            icoSphere1.get<rcube::Drawable>()->material);
        phong->diffuse_color = glm::vec3(0.0, 0.3, 0.7);
    }
    viewer.execute();
    return 0;
}