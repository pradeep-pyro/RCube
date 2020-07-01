#include "RCube/Core/Graphics/Materials/Material.h"

namespace rcube
{

void Material::use()
{
    if (shader_ != nullptr)
    {
        setUniforms();
        shader_->use();
    }
}
void Material::done()
{
    if (shader_ != nullptr)
    {
        shader_->done();
    }
}

RenderPriority Material::renderPriority() const
{
    return RenderPriority::Opaque;
}

const RenderSettings Material::renderState() const
{
    return RenderSettings{};
}

ShaderProgram *Material::shader()
{
    return (shader_ == nullptr)  ? nullptr : shader_.get();
};

} // namespace rcube