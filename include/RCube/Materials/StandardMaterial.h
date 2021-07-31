#pragma once

#include "RCube/Components/ForwardMaterial.h"
#include "RCube/Components/Camera.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"
#include <memory>

namespace rcube
{

class StandardMaterial : public ShaderMaterial
{
    std::vector<DrawCall::Texture2DInfo> textures_;
    std::vector<DrawCall::TextureCubemapInfo> cubemaps_;

  public:
    glm::vec3 albedo = glm::vec3(1.f);
    float roughness = 0.5f;
    float metallic = 0.5f;
    float opacity = 1.f;
    std::shared_ptr<Texture2D> albedo_texture = nullptr;
    std::shared_ptr<Texture2D> roughness_texture = nullptr;
    std::shared_ptr<Texture2D> metallic_texture = nullptr;
    std::shared_ptr<Texture2D> normal_texture = nullptr;
    std::shared_ptr<Texture2D> ibl_brdfLUT = nullptr;
    std::shared_ptr<TextureCubemap> ibl_irradiance = nullptr;
    std::shared_ptr<TextureCubemap> ibl_prefilter = nullptr;
    bool image_based_lighting = true;

    bool wireframe = false;
    float wireframe_thickness = 1.f;
    glm::vec3 wireframe_color = glm::vec3(0.f, 0.f, 0.f);

    StandardMaterial();
    void updateUniforms(std::shared_ptr<ShaderProgram> shader) override;
    const std::vector<DrawCall::Texture2DInfo> textureSlots() override;
    const std::vector<DrawCall::TextureCubemapInfo> cubemapSlots() override;
    void setIBLFromCamera(Camera *cam);
    void drawGUI() override;
};

} // namespace rcube