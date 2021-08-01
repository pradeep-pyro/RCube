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
    [[deprecated]] RenderSettings state_;
    const std::string name_;

  public:
    ShaderMaterial(const std::string &name);
    ~ShaderMaterial() = default;
    virtual void updateUniforms(std::shared_ptr<ShaderProgram> shader);
    virtual void drawGUI();
    virtual const std::vector<DrawCall::Texture2DInfo> textureSlots();
    virtual const std::vector<DrawCall::TextureCubemapInfo> cubemapSlots();
    [[deprecated]] const RenderSettings &state() const;
    const std::string &name() const;
    std::shared_ptr<ShaderMaterial> next_pass = nullptr;
};

/**
 * ForwardMaterial is the base class for all materials understood by the ForwardRenderSystem
 */
class ForwardMaterial : public Component<ForwardMaterial>
{
  public:
    float opacity = 1.f;
    std::shared_ptr<ShaderMaterial> shader = nullptr;
    void drawGUI();
};

} // namespace rcube