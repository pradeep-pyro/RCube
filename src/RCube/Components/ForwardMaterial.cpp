#include "RCube/Components/ForwardMaterial.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"

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
    if (shader != nullptr)
    {
        shader->drawGUI();
    }
    else
    {
        ImGui::Text("No valid shader found.");
    }
}

} // namespace rcube