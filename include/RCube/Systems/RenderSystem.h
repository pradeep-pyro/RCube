#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "glm/glm.hpp"

#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Graphics/OpenGL/Framebuffer.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"

namespace rcube
{

class RenderSystem : public System
{
  public:
    struct RenderSystemConfig
    {
        glm::ivec2 resolution = glm::ivec2(1280, 720);
        unsigned int msaa = 0;
    };

    RenderSystem(glm::ivec2 resolution = glm::ivec2(1280, 720), unsigned int msaa = 0);
    virtual ~RenderSystem() override = default;
    virtual void initialize() override;
    virtual void cleanup() override;
    virtual void update(bool force = false) override;
    virtual unsigned int priority() const override;

  protected:
    virtual void drawEntity(Entity ent);
    glm::ivec2 resolution_ = glm::ivec2(1280, 720);
    GLRenderer renderer;
    std::shared_ptr<Framebuffer> framebufferms_;
    std::shared_ptr<Framebuffer> framebuffer_;
    std::shared_ptr<Framebuffer> effect_framebuffer_;
    unsigned int msaa_;
};

} // namespace rcube

#endif // RENDERSYSTEM_H
