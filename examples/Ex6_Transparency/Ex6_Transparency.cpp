#include "RCube/Core/Graphics/MeshGen/Plane.h"
#include "RCubeViewer/Components/Pickable.h"
#include "RCubeViewer/RCubeViewer.h"

using namespace rcube;
using namespace viewer;

void addPlane(EntityHandle ent)
{
    ent.get<Drawable>()->mesh = Mesh::create(plane(1, 1, 1, 1, rcube::Orientation::NegativeZ));
    ent.get<Drawable>()->mesh->uploadToGPU();
}

void addMaterial(EntityHandle ent, const glm::vec3 color, float opacity)
{
    auto mat = std::make_shared<MatCapRGBMaterial>();
    mat->color = color;
    mat->opacity = opacity;
    ent.get<ForwardMaterial>()->shader = mat;
}

int main()
{
    // Create a viewer
    RCubeViewerProps props;
    props.ground_plane = false;
    props.background_color_bottom = glm::vec3(1.f);
    props.background_color_top = glm::vec3(1.f);
    viewer::RCubeViewer viewer(props);

    EntityHandle plane_r = viewer.addMeshEntity("Red plane");
    addPlane(plane_r);
    addMaterial(plane_r, glm::vec3(1, 0, 0), 0.5f);

    EntityHandle plane_g = viewer.addMeshEntity("Green plane");
    addPlane(plane_g);
    addMaterial(plane_g, glm::vec3(0, 1, 0), 0.5f);

    EntityHandle plane_b = viewer.addMeshEntity("Blue plane");
    addPlane(plane_b);
    addMaterial(plane_b, glm::vec3(0, 0, 1), 0.5f);

    plane_r.get<Transform>()->translate(glm::vec3(0, 0, 0.5f));
    plane_g.get<Transform>()->translate(glm::vec3(0, 0, 0));
    plane_b.get<Transform>()->translate(glm::vec3(0, 0, 1.f));

    // Show viewer
    viewer.execute();
    return 0;
}