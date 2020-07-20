#pragma once

#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/Framebuffer.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"

namespace rcube
{

// class GBuffer
//{
//    std::shared_ptr<Framebuffer> fbo_;
//    std::shared_ptr<Texture2D> textures_[4];
//
//  public:
//    enum GBufferTexture
//    {
//        POSITION_ROUGHNESS,
//        NORMALS_METALLIC,
//        ALBEDO,
//        DEPTH
//    };
//    static GBuffer create(size_t width, size_t height);
//    void useForWrite();
//    void useForRead();
//    void done();
//    void setReadBuffer(GBufferTexture texture);
//    bool isComplete() const;
//    std::shared_ptr<Framebuffer> fbo();
//};

class GBuffer
{
  public:
    enum GBUFFER_TEXTURE_TYPE
    {
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_NUM_TEXTURES
    };

    GBuffer() = default;

    ~GBuffer() = default;

    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);

    void BindForWriting();

    void BindForReading();

    void SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType);
  private:
    GLuint m_fbo;
    GLuint m_textures[GBUFFER_NUM_TEXTURES];
    GLuint m_depthTexture;
};

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
    std::shared_ptr<ShaderProgram> gbuffer_shader_;
    std::shared_ptr<ShaderProgram> lighting_shader_;
    std::shared_ptr<Framebuffer> framebuffer_hdr_;
    std::shared_ptr<Framebuffer> framebuffer2_;
    unsigned int msaa_;
};

} // namespace rcube
