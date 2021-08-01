#include "RCube/Core/Graphics/ShaderManager.h"
#include < iostream>

namespace rcube
{
std::vector<std::string>
ForwardRenderSystemShaderManager::prepareShaderSource(const std::string &src,
                                                      ForwardRenderPass pass)
{
    return {RCUBE_GLSL_VERSION_STR, defines_[static_cast<size_t>(pass)], src};
}
ForwardRenderSystemShaderManager &ForwardRenderSystemShaderManager::instance()
{
    static ForwardRenderSystemShaderManager state;
    return state;
}

void ForwardRenderSystemShaderManager::create(const std::string &name,
                                              const std::string &vertex_shader,
                                              const std::string &fragment_shader, bool debug)
{
    if (has(name))
    {
        return;
    }
    for (int i = 0; i < 2; ++i)
    {
        std::vector<std::string> vs = prepareShaderSource(vertex_shader, ForwardRenderPass(i));
        std::vector<std::string> fs = prepareShaderSource(fragment_shader, ForwardRenderPass(i));
        auto shader = ShaderProgram::create(vs, fs, debug);
        res_[{name, static_cast<ForwardRenderPass>(i)}] = shader;
    }
    return;
}

void ForwardRenderSystemShaderManager::create(const std::string &name,
                                              const std::string &vertex_shader,
                                              const std::string &geometry_shader,
                                              const std::string &fragment_shader, bool debug)
{
    if (has(name))
    {
        return;
    }
    for (int i = 0; i < 2; ++i)
    {
        std::vector<std::string> vs = prepareShaderSource(vertex_shader, ForwardRenderPass(i));
        std::vector<std::string> gs = prepareShaderSource(geometry_shader, ForwardRenderPass(i));
        std::vector<std::string> fs = prepareShaderSource(fragment_shader, ForwardRenderPass(i));
        auto shader = ShaderProgram::create(vs, gs, fs, debug);
        res_[{name, static_cast<ForwardRenderPass>(i)}] = shader;
    }
    return;
}

std::shared_ptr<ShaderProgram> ForwardRenderSystemShaderManager::get(const std::string &name,
                                                                     ForwardRenderPass pass)
{
    return res_.at({name, pass});
}

bool ForwardRenderSystemShaderManager::has(const std::string &name)
{
    auto it = std::find_if(res_.begin(), res_.end(),
                           [name](const auto &val) { return val.first.first == name; });
    return it != res_.end();
}

void ForwardRenderSystemShaderManager::clear()
{
    res_.clear();
}

} // namespace rcube