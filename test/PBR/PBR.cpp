#include "RCube/Core/Graphics/Effects/GammaCorrectionEffect.h"
#include "RCube/Core/Graphics/ImageBasedLighting/IBLDiffuse.h"
#include "RCube/Core/Graphics/ImageBasedLighting/IBLSpecularSplitSum.h"
#include "RCube/Core/Graphics/Materials/PhysicallyBasedMaterial.h"
#include "RCube/Core/Graphics/MeshGen/SuperShape.h"
#include "RCubeViewer/RCubeViewer.h"

#ifndef RESOURCE_PATH
#define RESOURCE_PATH "../../test/PBR"
#endif

int main()
{
    using namespace rcube;

    // Properties to configure the viewer
    viewer::RCubeViewerProps props;
    props.resolution = glm::vec2(1280 /*4096*/, 720 /*2160*/); // 720p
    props.resolution = glm::vec2(4096, 2160);                  // 4K
    props.MSAA = 2;                                            // turn on 2x multisampling
    props.ground_plane = false;                                // hide the ground plane

    // Create a viewer
    viewer::RCubeViewer viewer(props);

    // Load environment cubemap and set as skybox
    std::shared_ptr<TextureCubemap> env_map = TextureCubemap::create(2000, 2000, 1, true, TextureInternalFormat::sRGBA8);
    env_map->setData(0, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "px.jpg", 3));
    env_map->setData(1, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "nx.jpg", 3));
    env_map->setData(2, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "py.jpg", 3));
    env_map->setData(3, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "ny.jpg", 3));
    env_map->setData(4, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "pz.jpg", 3));
    env_map->setData(5, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "nz.jpg", 3));
    viewer.camera().get<Camera>()->skybox = env_map;
    viewer.camera().get<Camera>()->use_skybox = true;

    // Add a fancy supershape
    // The returned entity has 2 components in it: (1) a Drawable component holding the mesh and
    // material (a Blinn-Phong material is used by default), (2) a Transform component holding the
    // local position, and local orientation with respect to a parent transform
    EntityHandle s = viewer.addSurface(
        "superShape", superShape(1.f, 200, 200, 1.f, 1.f, 3.f, 6.f, 1.f, 1.f, 1.f));
    auto mat = std::dynamic_pointer_cast<PhysicallyBasedMaterial>(s.get<Drawable>()->material);
    mat->albedo = glm::vec3(0.953, 0.788, 0.408);
    mat->roughness = 0.1f;
    mat->metallic = 1.f;
    // Assign image based lighting textures to supershape's material
    // The IBL maps are precomputed as:
    // (1) diffuse irradiance cubemap using importance sampling, and
    // (2, 3) prefiltered specular cubmapand BRDF 2D LUT using the split-sum approximation.
    // mat->createIBLMaps(env_map);
    // This is not required in practice since the viewer updates all image-based lighting maps during initialization

    // Show viewer
    viewer.execute();
    return 0;
}