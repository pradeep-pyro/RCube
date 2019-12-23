#ifndef DIFFUSE_H
#define DIFFUSE_H

#include "RCube/Core/Graphics/OpenGL/Material.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"

namespace rcube
{

class DiffuseIrradianceShader : public Material
{
  public:
    int num_samples = 512;

    virtual ~DiffuseIrradianceShader() override = default;
    virtual std::string vertexShader() override;
    virtual std::string fragmentShader() override;
    virtual std::string geometryShader() override;
    virtual void setUniforms() override;
    virtual int renderPriority() const override;

    std::shared_ptr<TextureCubemap> environment_map;
};

class DiffusePrefilter
{
  public:
    int num_samples = 512;

    DiffusePrefilter(unsigned int resolution = 32);
    ~DiffusePrefilter();
    std::shared_ptr<TextureCubemap> prefilter(std::shared_ptr<TextureCubemap> env_map);

  private:
    DiffuseIrradianceShader shader_;
    unsigned int resolution_ = 32;
    GLRenderer rdr_;
    std::shared_ptr<Framebuffer> fbo_;
    std::shared_ptr<Mesh> cube_;
    glm::mat4 projection_;
    std::vector<glm::mat4> views_;
};

} // namespace rcube

#endif // DIFFUSE_IRRADIANCE_H
