#include "RCube/Components/PointLight.h"
#include "RCube/Components/DirectionalLight.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

namespace rcube
{

PointLight::PointLight(float radius, glm::vec3 rgb)
{
    setRadius(radius);
    setColor(rgb);
}

float PointLight::radius() const
{
    return radius_;
}

void PointLight::setRadius(float val)
{
    radius_ = val;
}

const glm::vec3 &PointLight::color() const
{
    return color_;
}

void PointLight::setColor(const glm::vec3 &rgb)
{
    color_ = rgb;
}

bool PointLight::castShadow() const
{
    return cast_shadow_;
}

void PointLight::setCastShadow(bool val)
{
    cast_shadow_ = val;
}

float PointLight::intensity() const
{
    return intensity_;
}

void PointLight::setIntensity(float val)
{
    intensity_ = val;
}

void PointLight::drawGUI()
{
    ImGui::ColorEdit3("Color", glm::value_ptr(color_));
    ImGui::InputFloat("Radius", &radius_);
    ImGui::InputFloat("Intensity", &intensity_);
    ImGui::Checkbox("Cast shadow", &cast_shadow_);
}

} // namespace rcube
