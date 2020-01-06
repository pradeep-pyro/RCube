#pragma once

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"

namespace rcube
{

class IBLSpecularSplitSum
{
    int num_samples_ = 512;
    unsigned int resolution_;
    std::shared_ptr<ShaderProgram> shader_;
    std::shared_ptr<ShaderProgram> shader_brdf_;
    GLRenderer rdr_;
    std::shared_ptr<Framebuffer> fbo_;
    std::shared_ptr<Mesh> cube_;
    glm::mat4 projection_;
    std::vector<glm::mat4> views_;

  public:
    IBLSpecularSplitSum(unsigned int resolution = 128, int num_samples = 512);
    ~IBLSpecularSplitSum();
    int numSamples() const;
    void setNumSamples(int num_samples);
    std::shared_ptr<TextureCubemap> prefilter(std::shared_ptr<TextureCubemap> env_map);
    std::shared_ptr<Texture2D> integrateBRDF(unsigned int resolution = 512);
};

} // namespace rcube
