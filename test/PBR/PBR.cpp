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
    // sRGBA format is necessary when loading textures from non-linear color space
    // formats like JPG so that OpenGL can convert them to linear space automatically.
    std::shared_ptr<TextureCubemap> env_map =
        TextureCubemap::create(2000, 2000, 1, true, TextureInternalFormat::sRGBA8);
    std::vector<std::string> filenames = {"px.jpg", "nx.jpg", "py.jpg",
                                          "ny.jpg", "pz.jpg", "nz.jpg"};
    for (int i = 0; i < 6; ++i)
    {
        env_map->setData(TextureCubemap::Side(i),
                         Image::fromFile(std::string(RESOURCE_PATH) + "/" + filenames[i], 3));
    }
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
    // Image based lighting
    // (1) diffuse irradiance cubemap using importance sampling, and
    // (2) prefiltered specular cubmap and
    // (3) BRDF 2D LUT using the split-sum approximation
    // are automatically precomputed based on
    // on the subemap's skybox when the viewer is initialized.
    // To manually update IBL, call:
    // viewer.updateImageBasedLighting);

    // Show viewer
    viewer.execute();
    return 0;
}