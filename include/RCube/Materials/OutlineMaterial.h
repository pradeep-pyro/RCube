#pragma once

#include "RCube/Components/ForwardMaterial.h"

namespace rcube
{

class OutlineMaterial : public ShaderMaterial
{
  public:
    glm::vec3 color = glm::vec3(0, 1, 1);
    float thickness = 2.f;
    float opacity = 1.f;

    OutlineMaterial();
    void updateUniforms() override;
    void drawGUI() override;
};

} // namespace rcube