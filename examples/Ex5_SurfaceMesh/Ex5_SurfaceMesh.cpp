#include "RCubeViewer/RCubeViewer.h"
#include "RCubeViewer/Components/Pickable.h"
#include "RCubeViewer/SurfaceMesh.h"
#include <random>

int main()
{
    using namespace rcube;
    using namespace viewer;

    // Create a viewer
    viewer::RCubeViewer viewer;

    // Add a subdivided icosahedron surface to viewer
    TriangleMeshData icosphere = icoSphere(1.0, 4);
    
    EntityHandle entity = viewer.addMeshEntity("Sphere");

    std::shared_ptr<SurfaceMesh> surf = SurfaceMesh::create(icosphere);
    entity.get<Drawable>()->mesh = std::static_pointer_cast<Mesh>(surf);

    // Create a per-vertex scalar field
    ScalarField height;
    std::vector<float> height_field;
    height_field.reserve(icosphere.vertices.size());
    for (auto &v : icosphere.vertices)
    {
        height_field.push_back(v.y);
    }
    height.setData(height_field);
    surf->addVertexScalarField("Height", height);
    
    // Create a per-face scalar field
    std::uniform_real_distribution<float> unif(0, 1);
    std::vector<float> rnd_field;
    std::mt19937_64 rng;
    for (auto &ind : icosphere.indices)
    {
        rnd_field.push_back(unif(rng));
    }
    ScalarField rnd;
    rnd.setData(rnd_field);
    surf->addFaceScalarField("Random", rnd);
    
    // Create a per-vertex vector field
    VectorField normals;
    normals.setVectors(icosphere.normals);
    surf->addVertexVectorField("Normals", normals);

    // Make the SurfaceMesh pickable with mouse click
    entity.add<Pickable>();

    // Show wireframe
    entity.get<Material>()->wireframe = true;

    // Show viewer
    viewer.execute();
    return 0;
}