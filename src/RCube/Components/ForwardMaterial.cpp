#include "RCube/Components/ForwardMaterial.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

namespace rcube
{
const RenderSettings &ShaderMaterial::state() const
{
    return state_;
}
std::shared_ptr<ShaderProgram> ShaderMaterial::get() const
{
    return shader_;
}
const std::vector<DrawCall::Texture2DInfo> ShaderMaterial::textureSlots()
{
    return {};
}
const std::vector<DrawCall::TextureCubemapInfo> ShaderMaterial::cubemapSlots()
{
    return {};
}
void ShaderMaterial::updateUniforms()
{
}
void ShaderMaterial::drawGUI()
{
}

void ForwardMaterial::drawGUI()
{
    ShaderMaterial *sh = shader.get();
    if (shader == nullptr)
    {
        ImGui::Text("No valid shader found.");
        return;
    }
    size_t pass = 1;
    while (sh != nullptr)
    {
        if (ImGui::CollapsingHeader(("Pass " + std::to_string(pass)).c_str(),
                                    ImGuiTreeNodeFlags_DefaultOpen))
        {
            sh->drawGUI();
        }
        sh = sh->next_pass.get();
        ++pass;
    }
}

} // namespace rcube