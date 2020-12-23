#include "RCube/Components/Drawable.h"
#include "imgui.h"

namespace rcube
{

void Drawable::drawGUI()
{
    // Visibility
    ImGui::Checkbox("Visible", &visible);

    // Mesh
    mesh->drawGUI();
}

} // namespace rcube