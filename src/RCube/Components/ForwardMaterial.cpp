#include "RCube/Components/ForwardMaterial.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

namespace rcube
{
const RenderSettings &ShaderMaterial::state() const
{
    return state_;
}
const std::string &ShaderMaterial::name() const
{
    return name_;
}
const std::vector<DrawCall::Texture2DInfo> ShaderMaterial::textureSlots()
{
    return {};
}
const std::vector<DrawCall::TextureCubemapInfo> ShaderMaterial::cubemapSlots()
{
    return {};
}
ShaderMaterial::ShaderMaterial(const std::string &name) : name_(name)
{
}
void ShaderMaterial::updateUniforms(std::shared_ptr<ShaderProgram> shader)
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
    ImGui::SliderFloat("Opacity", &opacity, 0.f, 1.f);
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