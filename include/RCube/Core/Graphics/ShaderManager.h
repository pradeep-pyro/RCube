#pragma once

#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#define RCUBE_GLSL_VERSION_STR "#version 450\n"

namespace rcube
{

enum class ForwardRenderPass
{
    Opaque,
    Transparent,
};

/**
 * ShaderManager is a class to create and manage shaders for the ForwardRenderSystem
 */
class ForwardRenderSystemShaderManager
{
    using ShaderNameAndPass = std::pair<std::string, ForwardRenderPass>;

    struct ShaderNameAndPassHash
    {
        std::size_t operator()(const ShaderNameAndPass &pair) const
        {
            return std::hash<std::string>()(pair.first) ^
                   std::hash<size_t>()(static_cast<size_t>(pair.second));
        }
    };
    std::unordered_map<ShaderNameAndPass, std::shared_ptr<ShaderProgram>, ShaderNameAndPassHash>
        res_;
    std::array<std::string, 2> defines_ = {"#define RCUBE_RENDERPASS 0\n",
                                           "#define RCUBE_RENDERPASS 1\n"};

    ForwardRenderSystemShaderManager() = default;
    ~ForwardRenderSystemShaderManager() = default;

    std::vector<std::string> prepareShaderSource(const std::string &src, ForwardRenderPass pass);

  public:
    ForwardRenderSystemShaderManager(const ForwardRenderSystemShaderManager &) = delete;

    ForwardRenderSystemShaderManager &operator=(const ForwardRenderSystemShaderManager &) = delete;

    static ForwardRenderSystemShaderManager &instance();

    void create(const std::string &name, const std::string &vertex_shader,
                const std::string &fragment_shader, bool debug = true);
    void create(const std::string &name, const std::string &vertex_shader,
                const std::string &geometry_shader, const std::string &fragment_shader,
                bool debug = true);
    std::shared_ptr<ShaderProgram> get(const std::string &name, ForwardRenderPass pass);
    bool has(const std::string &name);
    void clear();
};

} // namespace rcube