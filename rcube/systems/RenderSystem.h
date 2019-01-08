#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "glm/glm.hpp"
#include "../ecs/system.h"
#include "../render/Framebuffer.h"
#include "../render/Renderer.h"

namespace rcube {

class RenderSystem : public System {
public:
    GLRenderer renderer;

    RenderSystem(glm::ivec2 resolution=glm::ivec2(1280, 720));
    virtual ~RenderSystem() override = default;
    virtual void initialize() override;
    virtual void cleanup() override;
    virtual void update(bool force=false) override;
private:
    glm::ivec2 resolution_ = glm::ivec2(1280, 720);
    std::shared_ptr<Framebuffer> framebuffer_;
};

} // namespace rcube

#endif // RENDERSYSTEM_H
