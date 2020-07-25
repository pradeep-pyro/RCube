#pragma once

#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/Framebuffer.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"

namespace rcube
{
struct PBRMaterial
{
    glm::vec3 albedo = glm::vec3(0.5f, 0.5f, 0.5f);
    float roughness = 0.5f;
    float metallic = 0.5f;
    std::shared_ptr<Texture2D> albedo_texture = nullptr;
    std::shared_ptr<Texture2D> roughness_texture = nullptr;
    std::shared_ptr<Texture2D> metallic_texture = nullptr;
    std::shared_ptr<Texture2D> normal_texture = nullptr;
    bool wireframe = false;
    float wireframe_thickness = 1.f;
    glm::vec3 wireframe_color = glm::vec3(0.f, 0.f, 0.f);
    // Image-based lighting
    std::shared_ptr<TextureCubemap> irradiance_map = nullptr;
    std::shared_ptr<TextureCubemap> prefilter_map = nullptr;
    std::shared_ptr<Texture2D> brdf_map = nullptr;
};

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
    glm::ivec2 resolution_ = glm::ivec2(1280, 720);
    GLRenderer renderer_;
    std::shared_ptr<Framebuffer> gbuffer_;
    std::shared_ptr<Framebuffer> framebuffer_hdr_;
    std::shared_ptr<ShaderProgram> gbuffer_shader_;
    std::shared_ptr<ShaderProgram> lighting_shader_;
    std::shared_ptr<ShaderProgram> skybox_shader_;
    std::shared_ptr<Mesh> skybox_mesh_;
    unsigned int msaa_;
};

} // namespace rcube
