#include "RCube/Components/DirectionalLight.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

namespace rcube
{

const glm::vec3 &DirectionalLight::direction() const
{
    return direction_;
}

void DirectionalLight::setDirection(const glm::vec3 &dir)
{
    direction_ = dir;
}

const glm::vec3 &DirectionalLight::color() const
{
    return color_;
}

void DirectionalLight::setColor(const glm::vec3 &rgb)
{
    color_ = rgb;
}

bool DirectionalLight::castShadow() const
{
    return cast_shadow_;
}

void DirectionalLight::setCastShadow(bool val)
{
    cast_shadow_ = val;
}

float DirectionalLight::intensity() const
{
    return intensity_;
}

void DirectionalLight::setIntensity(float val)
{
    intensity_ = val;
}

void DirectionalLight::drawGUI()
{
    ImGui::ColorEdit3("Color", glm::value_ptr(color_));
    ImGui::InputFloat3("Direction", glm::value_ptr(direction_));
    ImGui::InputFloat("Intensity", &intensity_);
    ImGui::Checkbox("Cast shadow", &cast_shadow_);
}

} // namespace rcube
