#include "RCube/Components/Drawable.h"
#include "imgui.h"

namespace rcube
{

void Drawable::drawGUI()
{
    // Visibility
    ImGui::Checkbox("Visible", &visible);

    // Mesh
    static const char *current_attr = nullptr;
    if (ImGui::BeginCombo("Attribute", current_attr))
    {
        for (auto &kv : mesh->attributes())
        {
            bool is_selected = (current_attr == kv.first.c_str());
            if (ImGui::Selectable(kv.first.c_str(), is_selected))
            {
                current_attr = kv.first.c_str();
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    if (current_attr != nullptr)
    {
        ImGui::LabelText("Count",
                         std::to_string(mesh->attribute(current_attr)->size()).c_str());
        ImGui::LabelText("Dimension",
                         std::to_string(mesh->attribute(current_attr)->dim()).c_str());
        ImGui::LabelText("Layout location",
                         std::to_string(mesh->attribute(current_attr)->location()).c_str());
        bool checked = mesh->attributeEnabled(current_attr);
        if (ImGui::Checkbox("Active", &checked))
        {
            checked ? mesh->enableAttribute(current_attr)
                    : mesh->disableAttribute(current_attr);
        }
    }
    ImGui::LabelText(
        "#Faces", std::to_string(mesh->indices()->size() / mesh->primitiveDim()).c_str());
}

} // namespace rcube