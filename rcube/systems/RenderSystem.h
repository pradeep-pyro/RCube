#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "../ecs/system.h"
#include "../render/Framebuffer.h"
#include "../render/Renderer.h"
#include "../render/Effect.h"

namespace rcube {

class RenderSystem : public System {
public:
    GLRenderer renderer;

    RenderSystem();
    virtual ~RenderSystem() override = default;
    virtual void initialize() override;
    virtual void cleanup() override;
    virtual void update(bool force=false) override;
};

} // namespace rcube

#endif // RENDERSYSTEM_H
