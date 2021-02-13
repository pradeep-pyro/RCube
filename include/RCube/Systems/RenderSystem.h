#pragma once

#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/Framebuffer.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Components/Camera.h"

namespace rcube
{

class DeferredRenderSystem : public System
{
  public:
    DeferredRenderSystem(glm::ivec2 resolution = glm::ivec2(1280, 720));
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
    void setPointLightsUBO();
    void initializePostprocess();
    void geometryPass();
    void lightingPass(Camera *cam);
    void postprocessPass(Camera *cam);
    void finalPass(Camera *cam);

    glm::ivec2 resolution_ = glm::ivec2(1280, 720);
    GLRenderer renderer_;
    std::shared_ptr<Framebuffer> gbuffer_;
    std::shared_ptr<Framebuffer> framebuffer_hdr_;
    std::shared_ptr<Framebuffer> framebuffer_shadow_;
    std::shared_ptr<ShaderProgram> gbuffer_shader_;
    std::shared_ptr<ShaderProgram> lighting_shader_;
    std::shared_ptr<ShaderProgram> shadow_shader_;
    std::shared_ptr<Framebuffer> framebuffer_brightness_;
    std::shared_ptr<Framebuffer> framebuffer_blur_[2];
    std::shared_ptr<Framebuffer> framebuffer_pp_;
    std::shared_ptr<ShaderProgram> shader_brightness_;
    std::shared_ptr<ShaderProgram> shader_blur_;
    std::shared_ptr<ShaderProgram> shader_pp_;
    std::shared_ptr<Texture2D> shadow_atlas_;
    // Uniform buffer objects for camera and lights
    std::shared_ptr<Buffer<BufferType::Uniform>> ubo_camera_;
    std::shared_ptr<Buffer<BufferType::Uniform>> ubo_dirlights_;
    std::shared_ptr<Buffer<BufferType::Uniform>> ubo_pointlights_;
    unsigned int msaa_;
    // Buffers
    std::vector<float> dirlight_data_;
    std::vector<float> pointlight_data_;
};

} // namespace rcube
