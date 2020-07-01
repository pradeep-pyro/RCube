#pragma once

#include <memory>
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"

namespace rcube
{

class Material
{
  public:
    virtual ~Material() = default;
    void use();
    void done();
    virtual RenderPriority renderPriority() const;
    virtual const RenderSettings renderState() const;
    ShaderProgram *shader();
    virtual void drawGUI()
    {
    }

  protected:
    virtual void setUniforms()
    {
    }

    std::shared_ptr<ShaderProgram> shader_;
};

} // namespace rcube
