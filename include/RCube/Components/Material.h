#pragma once

#include "RCube/Core/Arch/Component.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"
#include <memory>
#include "glm/glm.hpp"

namespace rcube
{

class Material : public Component<Material>
{
  public:
    glm::vec3 albedo = glm::vec3(1.f);
    float roughness = 0.5f;
    float metallic = 0.5f;
    std::shared_ptr<Texture2D> albedo_texture = nullptr;
    std::shared_ptr<Texture2D> roughness_texture = nullptr;
    std::shared_ptr<Texture2D> metallic_texture = nullptr;
    std::shared_ptr<Texture2D> normal_texture = nullptr;
    bool wireframe = false;
    float wireframe_thickness = 1.f;
    glm::vec3 wireframe_color = glm::vec3(0.f, 0.f, 0.f);

    void drawGUI();
};

} // namespace rcube