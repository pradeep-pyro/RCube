#pragma once

#include <iostream>
#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Graphics/OpenGL/Framebuffer.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"

namespace rcube
{

class RenderSystem : public System
{
  public:
    RenderSystem(glm::ivec2 resolution = glm::ivec2(1280, 720), unsigned int msaa = 0);
    virtual ~RenderSystem() override = default;
    virtual void initialize() override;
    virtual void cleanup() override;
    virtual void update(bool force = false) override;
    virtual unsigned int priority() const override;
    virtual const std::string name() const override
    {
        return "RenderSystem";
    }
    int pick(int x, int y)
    {
        GLint pixel;
        checkGLError();
        picking_framebuffer_->use();
        checkGLError();
        glReadPixels(x, picking_framebuffer_->height() - y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &pixel);
        checkGLError();
        picking_framebuffer_->done();
        return pixel;
    }

  protected:
    virtual void drawEntity(Entity ent);
    glm::ivec2 resolution_ = glm::ivec2(1280, 720);
    GLRenderer renderer;
    std::shared_ptr<Framebuffer> framebufferms_;
    std::shared_ptr<Framebuffer> framebuffer_;
    std::shared_ptr<Framebuffer> effect_framebuffer_;
    std::shared_ptr<Framebuffer> picking_framebuffer_;
    unsigned int msaa_;
};

} // namespace rcube
