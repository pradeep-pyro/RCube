#pragma once

#include "RCube/Components/ForwardMaterial.h"

namespace rcube
{

class DepthMaterial : public ShaderMaterial
{
  public:
    float znear = 0.1f;
    float zfar = 100.f;

    DepthMaterial();
    void updateUniforms() override;
    void drawGUI() override;
};

} // namespace rcube