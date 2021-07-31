#pragma once

#include "RCube/Components/ForwardMaterial.h"

namespace rcube
{

class UnlitMaterial : public ShaderMaterial
{
  public:
    glm::vec3 color = glm::vec3(1, 1, 1);
    bool use_vertex_colors = false;
    float opacity = 1.f;

    UnlitMaterial();
    void updateUniforms(std::shared_ptr<ShaderProgram> shader) override;
    void drawGUI() override;
};

} // namespace rcube