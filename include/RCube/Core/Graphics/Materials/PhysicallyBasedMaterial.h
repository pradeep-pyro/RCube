#pragma once

#include "RCube/Core/Graphics/Materials/Material.h"
#include "RCube/Core/Graphics/Materials/constants.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include "glm/glm.hpp"
#include <string>

namespace rcube
{

/**
 * PhysicallyBasedMaterial is for displaying realistic surfaces with varying roughness and metalness
 * characteristics. It uses a physically-based Cook-Torrance BRDF for computing specular.
 */
class PhysicallyBasedMaterial : public Material
{
  public:
    glm::vec3 albedo = glm::vec3(0.5f, 0.5f, 0.5f);
    float roughness = 0.5f;
    float metallic = 0.5f;
    std::shared_ptr<Texture2D> albedo_texture = nullptr;
    std::shared_ptr<Texture2D> roughness_texture = nullptr;
    std::shared_ptr<Texture2D> metallic_texture = nullptr;
    std::shared_ptr<Texture2D> normal_texture = nullptr;
    bool wireframe = false;
    float wireframe_thickness = 1.f;
    glm::vec3 wireframe_color = glm::vec3(0.f, 0.f, 0.f);
    // Image-based lighting
    std::shared_ptr<TextureCubemap> irradiance_map = nullptr;
    std::shared_ptr<TextureCubemap> prefilter_map = nullptr;
    std::shared_ptr<Texture2D> brdf_map = nullptr;

    PhysicallyBasedMaterial();

    virtual void setUniforms() override;

    virtual void drawGUI() override;

    void setIBLMaps(std::shared_ptr<TextureCubemap> irradiance,
                    std::shared_ptr<TextureCubemap> prefilter, std::shared_ptr<Texture2D> brdf);

    void createIBLMaps(std::shared_ptr<TextureCubemap> environment_map);

    virtual const RenderSettings renderState() const;
};

} // namespace rcube
