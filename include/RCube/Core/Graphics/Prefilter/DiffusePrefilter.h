#ifndef DIFFUSE_H
#define DIFFUSE_H

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"

namespace rcube
{

class DiffusePrefilter
{
    int num_samples_ = 512;
    unsigned int resolution_ = 32;
    std::shared_ptr<ShaderProgram> shader_;
    GLRenderer rdr_;
    std::shared_ptr<Framebuffer> fbo_;
    std::shared_ptr<Mesh> cube_;
    glm::mat4 projection_;
    std::vector<glm::mat4> views_;

  public:
    DiffusePrefilter(unsigned int resolution = 32, int num_samples = 512);
    ~DiffusePrefilter();
    int numSamples() const;
    void setNumSamples(int num_samples);
    std::shared_ptr<TextureCubemap> prefilter(std::shared_ptr<TextureCubemap> env_map);
};

} // namespace rcube

#endif // DIFFUSE_IRRADIANCE_H
