#include "RCube/Components/DirectionalLight.h"
namespace rcube
{

DirectionalLight::DirectionalLight(glm::vec3 rgb)
{
    light_.pos_w = 0.f;
    light_.color = rgb;
    shadowmap_origin = glm::ivec2(0);
    shadowmap_size = glm::ivec2(1024);
}

const glm::vec3 &DirectionalLight::color() const
{
    return light_.color;
}

void DirectionalLight::setColor(const glm::vec3 &rgb)
{
    light_.color = rgb;
}

} // namespace rcube
