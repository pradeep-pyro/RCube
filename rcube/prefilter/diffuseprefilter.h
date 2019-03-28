#ifndef DIFFUSE_H
#define DIFFUSE_H

#include "../render/Material.h"
#include "../render/Mesh.h"
#include "../render/Renderer.h"
#include "../render/Texture.h"

namespace rcube {

class IrradianceShader : public Material {
public:
    virtual ~IrradianceShader() override = default ;
    virtual std::string vertexShader() override;
    virtual std::string fragmentShader() override;
    virtual std::string geometryShader() override;
    virtual void setUniforms() override;
    virtual int renderPriority() const override;

    std::shared_ptr<TextureCubemap> environment_map;
};

class DiffusePrefilter {
public:
    DiffusePrefilter(unsigned int resolution=32);
    ~DiffusePrefilter();
    std::shared_ptr<TextureCubemap> prefilter(std::shared_ptr<TextureCubemap> env_map);
private:
    IrradianceShader shader_;
    unsigned int resolution_ = 32;
    GLRenderer rdr_;
    std::shared_ptr<Framebuffer> fbo_;
    std::shared_ptr<Mesh> cube_;
    glm::mat4 projection_;
    std::vector<glm::mat4> views_;
};

std::shared_ptr<TextureCubemap>
prefilterDiffuseIrradiance(std::shared_ptr<TextureCubemap> env_map,
                           unsigned int resolution=32);

} // namespace rcube

#endif // DIFFUSE_IRRADIANCE_H
