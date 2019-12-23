#include "RCube/Components/PointLight.h"

namespace rcube
{

PointLight::PointLight(float radius, glm::vec3 rgb)
{
    setRadius(radius);
    light_.color = rgb;
    light_.cone_angle = glm::pi<float>();
    light_.pos_w = 1.f; // homogenous coordinate of position
}

float PointLight::radius() const
{
    return light_.radius;
}

void PointLight::setRadius(float val)
{
    light_.radius = val;
}

const glm::vec3 &PointLight::color() const
{
    return light_.color;
}

void PointLight::setColor(const glm::vec3 &rgb)
{
    light_.color = rgb;
}

} // namespace rcube
