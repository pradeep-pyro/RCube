#include "RCube/Core/Graphics/OpenGL/Material.h"
namespace rcube
{

Material::Material() : shader_(std::make_shared<ShaderProgram>()), init_(false)
{
}

Material::~Material()
{
}

void Material::initialize()
{
    if (init_)
    {
        return;
    }
    VertexShader vert = vertexShader();
    FragmentShader frag = fragmentShader();
    GeometryShader geom = geometryShader();
    if (geom.source.size() > 0)
    {
        // shader_ = ShaderProgram::create(vert_src, geom_src, frag_src, true);
        shader_ = ShaderProgram::create(vert, geom, frag, true);
    }
    else
    {
        // shader_ = ShaderProgram::create(vert_src, frag_src, true);
        shader_ = ShaderProgram::create(vert, frag, true);
    }
}

std::shared_ptr<ShaderProgram> Material::shader() const
{
    return shader_;
}

void Material::use()
{
    shader_->use();
    setUniforms();
}

void Material::done()
{
    shader_->done();
}

} // namespace rcube
