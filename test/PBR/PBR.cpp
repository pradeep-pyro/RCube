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
    props.MSAA = 2;                                            // turn on 2x multisampling
    props.ground_plane = false;                                // hide the ground plane

    // Create a viewer
    viewer::RCubeViewer viewer(props);

    // Load environment cubemap and set as skybox
    std::shared_ptr<TextureCubemap> env_map = TextureCubemap::create(2000, 2000); //, 1, true, TextureInternalFormat::sRGBA8);
    env_map->setData(0, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "px.jpg", 3));
    env_map->setData(1, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "nx.jpg", 3));
    env_map->setData(2, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "py.jpg", 3));
    env_map->setData(3, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "ny.jpg", 3));
    env_map->setData(4, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "pz.jpg", 3));
    env_map->setData(5, Image::fromFile(std::string(RESOURCE_PATH) + "/" + "nz.jpg", 3));
    
    // Precompute image-based lighting cubemaps for indirect lighting in PBR shader
    // Compute the diffuse irradiance cubemap by importance sampling
    std::shared_ptr<TextureCubemap> irradiance_map;
    std::shared_ptr<TextureCubemap> prefilter_map;
    std::shared_ptr<Texture2D> brdf_lut;
    
        IBLDiffuse ibl_diffuse;
        irradiance_map = ibl_diffuse.irradiance(env_map);
        // Compute the specular prefilter and integrated BRDF using Epic Games split-sum
        // approximation
        IBLSpecularSplitSum ibl_specular;
        prefilter_map = ibl_specular.prefilter(env_map);
        brdf_lut = ibl_specular.integrateBRDF();
    
        viewer.camera().get<Camera>()->skybox = prefilter_map;
        viewer.camera().get<Camera>()->use_skybox = true;


    // Add a fancy supershape
    // The returned entity has 2 components in it: (1) a Drawable component holding the mesh and
    // material (a Blinn-Phong material is used by default), (2) a Transform component holding the
    // local position, and local orientation with respect to a parent transform
    /*EntityHandle s = viewer.addSurface(
        "superShape", superShape(1.f, 200, 200, 1.f, 1.f, 7.f, 7.f, 0.2f, 1.7f, 1.7f));*/
    EntityHandle s = viewer.addIcoSphereSurface("sphere", 1.0, 4);
    s.get<Drawable>()->material = makePhysicallyBasedMaterial(glm::vec3(1.f));

    // Assign image based lighting textures to supershape's material
    std::shared_ptr<ShaderProgram> &material = s.get<Drawable>()->material;
    material->cubemap("irradiance_map") = irradiance_map;
    material->cubemap("prefilter_map") = prefilter_map;
    material->texture("brdf_lut") = brdf_lut;
    material->uniform("use_image_based_lighting").set(true);

    // Apply gamma correction to the screen
    viewer.camera().get<Camera>()->postprocess.push_back(makeGammaCorrectionEffect());

    // Show viewer
    viewer.execute();
    return 0;
}