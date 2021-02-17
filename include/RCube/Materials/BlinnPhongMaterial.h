#pragma once

#include "RCube/Components/ForwardMaterial.h"

namespace rcube
{

class MatCapMaterial : public ShaderMaterial
{
  public:
    bool blend_rgb = true;

    MatCapMaterial();
    void updateUniforms() override;
    void drawGUI() override;
};

} // namespace rcube