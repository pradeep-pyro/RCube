#include "RCube/Core/Graphics/MeshGen/Cone.h"
#include "RCube/Core/Graphics/MeshGen/Obj.h"
#include "RCubeViewer/Colormap.h"
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

    // Make pointcloud pickable with mouse click
    entity.add<Pickable>();

    // Custom window to show picked point
    viewer.customGUI = [&](RCubeViewer &v) {
        ImGui::Begin("Pick");
        Pickable *pick_comp = entity.get<Pickable>();
        if (pick_comp->picked)
        {
            size_t index = pick_comp->primitive;
            ImGui::LabelText("Picked point", std::to_string(index).c_str());
            // Get the surface mesh
            std::shared_ptr<SurfaceMesh> pc =
                std::dynamic_pointer_cast<SurfaceMesh>(entity.get<Drawable>()->mesh);

            //// Display the fields values for the picked vertex/face
            ImGui::Separator();
            ImGui::LabelText("Height", std::to_string(surf->vertexScalarField("Height").data()[index]).c_str());
            ImGui::Separator();
            const glm::vec3 nor = pc->vertexVectorField("Normals").vectors()[index];
            ImGui::LabelText("Normal.x", std::to_string(nor.x).c_str());
            ImGui::LabelText("Normal.y", std::to_string(nor.y).c_str());
            ImGui::LabelText("Normal.z", std::to_string(nor.z).c_str());
        }
        ImGui::End();
    };

    // Show viewer
    viewer.execute();
    return 0;
}