#pragma once

#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/Framebuffer.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Components/Camera.h"

namespace rcube
{

class ForwardRenderSystem : public System
{
  public:
    ForwardRenderSystem(glm::ivec2 resolution = glm::ivec2(1280, 720), unsigned int msaa = 0);
    virtual ~ForwardRenderSystem() override = default;
    virtual void initialize() override;
    virtual void cleanup() override;
    virtual void update(bool force = false) override;
    virtual unsigned int priority() const override;
    virtual const std::string name() const override
    {
        return "ForwardRenderSystem";
    }

  protected:
    void setCameraUBO(const glm::vec3 &eye_pos, const glm::mat4 &world_to_view,
                      const glm::mat4 &view_to_projection, const glm::mat4 &projection_to_viewport);
    void setDirectionalLightsUBO();
    void setPointLightsUBO();
    void initializePostprocess();
    void renderOpaqueGeometry(Camera *cam);
    void renderTransparentGeometry(Camera *cam);
    void postprocessPass(Camera *cam);
    void finalPass(Camera *cam);

    glm::ivec2 resolution_ = glm::ivec2(1280, 720);
    GLRenderer renderer_;
    std::shared_ptr<Framebuffer> gbuffer_;
    std::shared_ptr<Framebuffer> framebuffer_hdr_;
    std::shared_ptr<Framebuffer> framebuffer_brightness_;
    std::shared_ptr<Framebuffer> framebuffer_blur_[2];
    std::shared_ptr<Framebuffer> framebuffer_pp_;
    std::shared_ptr<ShaderProgram> shader_standard_;
    std::shared_ptr<ShaderProgram> shader_brightness_;
    std::shared_ptr<ShaderProgram> shader_blur_;
    std::shared_ptr<ShaderProgram> shader_pp_;
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