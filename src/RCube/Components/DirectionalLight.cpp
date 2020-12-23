#include "RCube/Components/DirectionalLight.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

namespace rcube
{

void DirectionalLight::drawGUI()
{
    ImGui::ColorEdit3("Color", glm::value_ptr(color));
    ImGui::InputFloat3("Direction", glm::value_ptr(direction));
    ImGui::InputFloat("Intensity", &intensity);
    ImGui::Checkbox("Cast shadow", &cast_shadow);
}

} // namespace rcube
