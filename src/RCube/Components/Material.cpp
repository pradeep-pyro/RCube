#include "RCube/Components/Material.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"

namespace rcube
{

void Material::drawGUI()
{
    ImGui::ColorEdit3("Albedo", glm::value_ptr(albedo));
    ImGui::SliderFloat("Roughness", &roughness, 0.04f, 1.f);
    ImGui::SliderFloat("Metallic", &metallic, 0.f, 1.f);
    ImGui::Text("Wireframe");
    ImGui::Checkbox("Show", &wireframe);
    ImGui::InputFloat("Thickness", &wireframe_thickness);
    ImGui::ColorEdit3("Color", glm::value_ptr(wireframe_color));
}

} // namespace rcube