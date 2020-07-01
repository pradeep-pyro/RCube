#pragma once

#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include "RCube/Core/Graphics/Materials/Material.h"

namespace rcube
{

/**
 * FlatMaterial is for representing objects in a flat style without any 3D shading
 */
class FlatMaterial : public Material
{
  public:
    FlatMaterial();

    virtual void setUniforms() override;

    virtual void drawGUI() override;

    virtual const RenderSettings renderState() const;
};

} // namespace rcube
