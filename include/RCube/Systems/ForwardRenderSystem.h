#pragma once

#include "RCube/Components/Camera.h"
#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/Framebuffer.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"

namespace rcube
{

class WeightedBlendedOITManager
{
    std::shared_ptr<Framebuffer> fbo_;
    std::shared_ptr<Texture2D> accum_tex_;
    std::shared_ptr<Texture2D> revealage_tex_;
    std::shared_ptr<ShaderProgram> composite_shader_;
    glm::ivec2 resolution_ = glm::ivec2(1280, 720);

  public:
    WeightedBlendedOITManager() = default;
    void initialize(glm::ivec2 resolution, std::shared_ptr<Texture2D> opaque_depth);
    std::shared_ptr<Framebuffer> getFramebuffer();
    std::shared_ptr<Texture2D> getAccumTexture();
    std::shared_ptr<Texture2D> getRevealageTexture();
    std::shared_ptr<ShaderProgram> getCompositeShader();
    void prepareTransparentPass(RenderTarget &rt, RenderSettings &state);
    void prepareCompositePass(std::shared_ptr<Framebuffer> opaque_fbo, RenderTarget &rt,
                              RenderSettings &state);
    void cleanup();
};

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
    std::shared_ptr<Texture2D> objPrimIDTexture() const
    {
        if (framebuffer_pick_ == nullptr)
        {
            return nullptr;
        }
        return framebuffer_pick_->colorAttachment(0);
    }
    Image screenshot() const
    {
        if (framebuffer_pp_ == nullptr)
        {
            return Image();
        }
        return framebuffer_pp_->getImage();
    }

  protected:
    void setCameraUBO(const glm::vec3 &eye_pos, const glm::mat4 &world_to_view,
                      const glm::mat4 &view_to_projection, const glm::mat4 &projection_to_viewport);
    void setDirectionalLightsUBO();
    void setPointLightsUBO();
    void initializePostprocess();
    void shadowMapPass();
    void depthPrepass(Camera *cam);
    void opaqueGeometryPass(Camera *cam);
    void transparentGeometryPass(Camera *cam);
    void pickFBOPass(Camera *cam);
    void postprocessPass(Camera *cam);
    void finalPass(Camera *cam);

    glm::ivec2 resolution_ = glm::ivec2(1280, 720);
    GLRenderer renderer_;
    std::shared_ptr<Framebuffer> framebuffer_hdr_;
    std::shared_ptr<Framebuffer> framebuffer_hdr_ms_;
    std::shared_ptr<Framebuffer> framebuffer_brightness_;
    std::shared_ptr<Framebuffer> framebuffer_blur_[2];
    std::shared_ptr<Framebuffer> framebuffer_pp_;
    std::shared_ptr<Framebuffer> framebuffer_shadow_;
    std::shared_ptr<Framebuffer> framebuffer_pick_;
    std::shared_ptr<Framebuffer> framebuffer_depth_;
    std::shared_ptr<Texture2D> depth_;
    std::shared_ptr<Framebuffer> framebuffer_depth_ms_;
    std::shared_ptr<Texture2D> depth_ms_;
    std::shared_ptr<Texture2D> shadow_atlas_;
    std::shared_ptr<ShaderProgram> shader_brightness_;
    std::shared_ptr<ShaderProgram> shader_blur_;
    std::shared_ptr<ShaderProgram> shader_shadow_;
    std::shared_ptr<ShaderProgram> shader_pp_;
    std::shared_ptr<ShaderProgram> shader_picking_;
    std::shared_ptr<ShaderProgram> shader_depth_;
    // Uniform buffer objects for camera and lights
    std::shared_ptr<Buffer<BufferType::Uniform>> ubo_camera_;
    std::shared_ptr<Buffer<BufferType::Uniform>> ubo_dirlights_;
    std::shared_ptr<Buffer<BufferType::Uniform>> ubo_pointlights_;
    unsigned int msaa_;
    // Buffers
    std::vector<float> dirlight_data_;
    std::vector<float> pointlight_data_;
    // Pick pass
    bool pick_pass_ = true;
    // Transparency
    WeightedBlendedOITManager wboit_;
};

} // namespace rcube