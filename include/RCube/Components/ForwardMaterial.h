#pragma once

#include "RCube/Core/Arch/Component.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include <functional>

namespace rcube
{

class ShaderMaterial
{
  protected:
    std::shared_ptr<ShaderProgram> shader_;
    RenderSettings state_;

  public:
    ~ShaderMaterial() = default;
    virtual void updateUniforms();
    virtual void drawGUI();
    virtual const std::vector<DrawCall::Texture2DInfo> textureSlots();
    virtual const std::vector<DrawCall::TextureCubemapInfo> cubemapSlots();
    const RenderSettings &state() const;
    std::shared_ptr<ShaderProgram> get() const;
    std::shared_ptr<ShaderMaterial> next_pass = nullptr;
};

/**
 * ForwardMaterial is the base class for all materials understood by the ForwardRenderSystem
 */
class ForwardMaterial : public Component<ForwardMaterial>
{
  public:
    std::shared_ptr<ShaderMaterial> shader = nullptr;
    void drawGUI();
};

} // namespace rcube