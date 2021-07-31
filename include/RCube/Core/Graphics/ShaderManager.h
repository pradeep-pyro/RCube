#pragma once

#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include <bitset>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#define RCUBE_GLSL_VERSION_STR "#version 450\n"

namespace rcube
{

using ShaderFeatures = std::bitset<8>;

class ShaderManager
{
    using ShaderNameAndFeatures = std::pair<std::string, ShaderFeatures>;

    struct ShaderNameAndFeaturesHash
    {
        std::size_t operator()(const ShaderNameAndFeatures &pair) const
        {
            return std::hash<std::string>()(pair.first) ^ std::hash<ShaderFeatures>()(pair.second);
        }
    };

    std::unordered_map<ShaderNameAndFeatures, std::shared_ptr<ShaderProgram>,
                       ShaderNameAndFeaturesHash>
        res_;

    ShaderManager() = default;
    ~ShaderManager() = default;

  public:
    ShaderManager(const ShaderManager &) = delete;

    ShaderManager &operator=(const ShaderManager &) = delete;

    static ShaderManager &instance();

    void create(const std::string &name, const std::string &vertex_shader,
                const std::string &fragment_shader, bool debug = true);
    void create(const std::string &name, const std::string &vertex_shader,
                const std::string &fragment_shader, const std::map<uint8_t, std::string> &defines,
                bool debug = true);
    void create(const std::string &name, const std::string &vertex_shader,
                const std::string &geometry_shader, const std::string &fragment_shader,
                bool debug = true);
    void create(const std::string &name, const std::string &vertex_shader,
                const std::string &geometry_shader, const std::string &fragment_shader,
                const std::map<uint8_t, std::string> &defines, bool debug = true);
    std::shared_ptr<ShaderProgram> get(const std::string &name);
    std::shared_ptr<ShaderProgram> get(const std::string &name, const ShaderFeatures &features);
    void clear();
};

} // namespace rcube