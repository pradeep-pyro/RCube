#include "RCube/Core/Graphics/MeshGen/Cone.h"
#include "RCube/Core/Graphics/MeshGen/Obj.h"
#include "RCubeViewer/Colormap.h"
#include "RCubeViewer/RCubeViewer.h"
#include "RCubeViewer/SurfaceMesh.h"

int main()
{
    using namespace rcube;
    using namespace viewer;

    // Properties to configure the viewer
    viewer::RCubeViewerProps props;
    props.resolution = glm::vec2(1280 /*4096*/, 720 /*2160*/); // 720p
    props.MSAA = 2;                                            // turn on 2x multisampling

    // Create a viewer
    viewer::RCubeViewer viewer(props);

    // Add a subdivided icosahedron surface to viewer
    TriangleMeshData icosphere = icoSphere(1.0, 4);
    EntityHandle entity = viewer.addSurface("Sphere", icosphere);

    std::shared_ptr<SurfaceMesh> surf = SurfaceMesh::create(icosphere);
    entity.get<Drawable>()->mesh = surf;

    // Create a per-vertex scalar field
    std::vector<float> height_field;
    height_field.reserve(icosphere.vertices.size());
    for (auto &v : icosphere.vertices)
    {
        height_field.push_back(v.y);
    }
    ScalarField height;
    height.setData(height_field);
    surf->addScalarField("Height", height);

    // Create a per-vertex vector field
    VectorField normals;
    normals.setVectors(icosphere.normals);
    surf->addVertexVectorField("Normals", normals);

    // Show viewer
    viewer.execute();
    return 0;
}