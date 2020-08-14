#pragma once

#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/Framebuffer.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"

namespace rcube
{

class DeferredRenderSystem : public System
{
  public:
    DeferredRenderSystem(glm::ivec2 resolution = glm::ivec2(1280, 720), unsigned int msaa = 0);
    virtual ~DeferredRenderSystem() override = default;
    virtual void initialize() override;
    virtual void cleanup() override;
    virtual void update(bool force = false) override;
    virtual unsigned int priority() const override;
    virtual const std::string name() const override
    {
        return "DeferredRenderSystem";
    }

  protected:
    void setCameraUBO(const glm::vec3 &eye_pos, const glm::mat4 &world_to_view,
                      const glm::mat4 &view_to_projection, const glm::mat4 &projection_to_viewport);
    void setDirectionalLightsUBO();

    glm::ivec2 resolution_ = glm::ivec2(1280, 720);
    GLRenderer renderer_;
    std::shared_ptr<Framebuffer> gbuffer_;
    std::shared_ptr<Framebuffer> framebuffer_hdr_;
    std::shared_ptr<Framebuffer> framebuffer_shadow_;
    std::shared_ptr<ShaderProgram> gbuffer_shader_;
    std::shared_ptr<ShaderProgram> lighting_shader_;
    std::shared_ptr<ShaderProgram> skybox_shader_;
    std::shared_ptr<ShaderProgram> shadow_shader_;
    std::shared_ptr<Texture2D> shadow_atlas_;
    // Uniform buffer objects for camera and lights
    std::shared_ptr<Buffer<BufferType::Uniform>> ubo_camera_;
    std::shared_ptr<Buffer<BufferType::Uniform>> ubo_dirlights_;
    unsigned int msaa_;
};

} // namespace rcube
