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
    direction_ = glm::normalize(dir);
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

glm::mat4 DirectionalLight::viewProjectionMatrix(float left, float right, float bottom, float top,
                                                 float znear, float zfar) const
{
    const glm::mat4 light_proj = glm::ortho<float>(left, right, bottom, top, znear, zfar);
    const glm::vec3 dir = direction();
    const glm::vec3 up = glm::length(glm::cross(dir, glm::vec3(0, 1, 0))) > 1e-6
                             ? glm::vec3(0, 1, 0)
                             : glm::vec3(1, 0, 0);
    const glm::mat4 light_view = glm::lookAt(-dir, glm::vec3(0, 0, 0), up);
    return light_proj * light_view;
}

void DirectionalLight::drawGUI()
{
    ImGui::ColorEdit3("Color", glm::value_ptr(color_));
    if (ImGui::InputFloat3("Direction", glm::value_ptr(direction_)))
    {
        direction_ = glm::normalize(direction_);
    }
    ImGui::InputFloat("Intensity", &intensity_);
    ImGui::Checkbox("Cast shadow", &cast_shadow_);
}

} // namespace rcube
