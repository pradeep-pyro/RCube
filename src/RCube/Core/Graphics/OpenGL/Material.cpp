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
    std::string vert_src = vertexShader();
    std::string frag_src = fragmentShader();
    std::string geom_src = geometryShader();
    if (geom_src.size() > 0)
    {
        shader_ = ShaderProgram::create(vert_src, geom_src, frag_src, true);
    }
    else
    {
        shader_ = ShaderProgram::create(vert_src, frag_src, true);
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

GridMaterial::GridMaterial() : shader_(std::make_shared<ShaderProgram>()), init_(false)
{
}

GridMaterial::~GridMaterial()
{
}

void GridMaterial::initialize()
{
    if (init_)
    {
        return;
    }
    std::string vert_src = vertexShader();
    std::string frag_src = fragmentShader();
    std::string geom_src = geometryShader();
    if (geom_src.size() > 0)
    {
        shader_ = ShaderProgram::create(vert_src, geom_src, frag_src, true);
    }
    else
    {
        shader_ = ShaderProgram::create(vert_src, frag_src, true);
    }
}

std::shared_ptr<ShaderProgram> GridMaterial::shader() const
{
    return shader_;
}

void GridMaterial::use()
{
    shader_->use();
    setUniforms();
}

void GridMaterial::done()
{
    shader_->done();
}

} // namespace rcube
