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
    ScalarField sf_rnd;
    sf_rnd.setData(face_scalar_field);
    surf->addFaceScalarField("Random", sf_rnd);
    VectorField vf_rnd;
    vf_rnd.setVectors(face_vector_field);
    surf->addFaceVectorField("Random", vf_rnd);
    
    // Create a per-vertex normal vector field
    VectorField normals;
    normals.setVectors(icosphere.normals);
    surf->addVertexVectorField("Normals", normals);

    // Create a per-face vector field
    VectorField rnd_vec_field;
    std::vector<glm::vec3> rnd_vecs;
    rnd_vecs.reserve(icosphere.indices.size());
    rnd_vec_field.setVectors(rnd_vecs);
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