#include "RCubeViewer/Components/Pickable.h"
#include "RCubeViewer/RCubeViewer.h"
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
    entity.get<ForwardMaterial>()->shader = std::make_shared<StandardMaterial>();
    std::shared_ptr<SurfaceMesh> surf = SurfaceMesh::create(icosphere);
    entity.get<Drawable>()->mesh = std::static_pointer_cast<Mesh>(surf);

    // Create a per-vertex scalar field
    std::vector<float> height_field;
    height_field.reserve(icosphere.vertices.size());
    for (auto &v : icosphere.vertices)
    {
        height_field.push_back(v.y);
    }
    surf->addVertexScalarField("Height");
    surf->vertexScalarField("Height").setData(height_field);

    // Create random per-face scalar and vector fields
    std::uniform_real_distribution<float> unif(0, 1);
    std::vector<float> face_scalar_field;
    std::vector<glm::vec3> face_vector_field;
    std::mt19937_64 rng;
    for (auto &ind : icosphere.indices)
    {
        face_scalar_field.push_back(unif(rng));
        face_vector_field.push_back(glm::vec3(unif(rng), unif(rng), unif(rng)));
    }
    surf->addFaceScalarField("Random");
    surf->faceScalarField("Random").setData(face_scalar_field);
    surf->addFaceVectorField("Random");
    surf->faceVectorField("Random").setVectors(face_vector_field);

    // Create a per-vertex normal vector field
    surf->addVertexVectorField("Normals");
    surf->vertexVectorField("Normals").setVectors(icosphere.normals);

    // Make the SurfaceMesh pickable with mouse click
    entity.add<Pickable>();

    // Show viewer
    viewer.execute();
    return 0;
}