#pragma once

#include "RCube/Components/ForwardMaterial.h"

namespace rcube
{

/**
 * The RGB blending equation and sphere images are attributed to Polyscope: https://polyscope.run/features/materials
 */
class MatCapRGBMaterial : public ShaderMaterial
{
    std::vector<DrawCall::Texture2DInfo> textures_;
    std::shared_ptr<Texture2D> red_;
    std::shared_ptr<Texture2D> green_;
    std::shared_ptr<Texture2D> blue_;
    std::shared_ptr<Texture2D> black_;

  public:
    glm::vec3 color = glm::vec3(1, 1, 1);
    glm::vec3 emissive_color = glm::vec3(0, 0, 0);
    bool wireframe = false;
    float wireframe_thickness = 1.f;
    glm::vec3 wireframe_color = glm::vec3(0, 0, 0);

    MatCapRGBMaterial();
    void updateUniforms(std::shared_ptr<ShaderProgram> shader) override;
    const std::vector<DrawCall::Texture2DInfo> textureSlots() override;
    void drawGUI() override;
};

class MatCapMaterial : public ShaderMaterial
{
    std::vector<DrawCall::Texture2DInfo> textures_;

  public:
    glm::vec3 color = glm::vec3(1, 1, 1);
    bool wireframe = false;
    float wireframe_thickness = 1.f;
    glm::vec3 wireframe_color = glm::vec3(0, 0, 0);
    std::shared_ptr<Texture2D> matcap;

    MatCapMaterial();
    void updateUniforms(std::shared_ptr<ShaderProgram> shader) override;
    const std::vector<DrawCall::Texture2DInfo> textureSlots() override;
    void drawGUI() override;
};

} // namespace rcube