#include "RCube/Core/Graphics/ShaderManager.h"

namespace rcube
{

ShaderManager &ShaderManager::instance()
{
    static ShaderManager state;
    return state;
}

void ShaderManager::create(const std::string &name, const std::string &vertex_shader,
                           const std::string &fragment_shader, bool debug)
{
    return create(name, vertex_shader, fragment_shader, std::map<uint8_t, std::string>(), debug);
}

void ShaderManager::create(const std::string &name, const std::string &vertex_shader,
                           const std::string &fragment_shader,
                           const std::map<uint8_t, std::string> &defines, bool debug)
{
    int powerset_size = 2 << static_cast<int>(defines.size());
    // This loops over all shader permutations
    for (uint8_t i = 0; i < powerset_size; ++i)
    {
        ShaderFeatures feat;
        std::vector<std::string> vs;
        std::vector<std::string> fs;
        vs.push_back(RCUBE_GLSL_VERSION_STR);
        fs.push_back(RCUBE_GLSL_VERSION_STR);
        feat.reset();
        for (uint8_t j = 0; j < (uint8_t)defines.size(); ++j)
        {
            // If bit j is set in i, then include the #define
            if (i & (1 << j))
            {
                feat.set(j);
                vs.push_back("#define " + defines.at(j) + "\n");
                fs.push_back("#define " + defines.at(j) + "\n");
            }
        }
        vs.push_back(vertex_shader);
        fs.push_back(fragment_shader);
        res_[{name, feat}] = ShaderProgram::create(vs, fs, debug);
    }
}

void ShaderManager::create(const std::string &name, const std::string &vertex_shader,
                           const std::string &geometry_shader, const std::string &fragment_shader,
                           bool debug)
{
    return create(name, vertex_shader, geometry_shader, fragment_shader,
                  std::map<uint8_t, std::string>(), debug);
}

void ShaderManager::create(const std::string &name, const std::string &vertex_shader,
                           const std::string &geometry_shader, const std::string &fragment_shader,
                           const std::map<uint8_t, std::string> &defines, bool debug)
{
    int powerset_size = 2 << static_cast<int>(defines.size());
    // This loops over all shader permutations
    for (uint8_t i = 0; i < powerset_size; ++i)
    {
        ShaderFeatures feat;
        std::vector<std::string> vs;
        std::vector<std::string> fs;
        std::vector<std::string> gs;
        vs.push_back(RCUBE_GLSL_VERSION_STR);
        fs.push_back(RCUBE_GLSL_VERSION_STR);
        gs.push_back(RCUBE_GLSL_VERSION_STR);
        feat.reset();
        for (uint8_t j = 0; j < (uint8_t)defines.size(); ++j)
        {
            // If bit j is set in i, then include the #define
            if (i & (1 << j))
            {
                feat.set(j);
                vs.push_back("#define " + defines.at(j) + "\n");
                fs.push_back("#define " + defines.at(j) + "\n");
                gs.push_back("#define " + defines.at(j) + "\n");
            }
        }
        vs.push_back(vertex_shader);
        gs.push_back(geometry_shader);
        fs.push_back(fragment_shader);
        auto shader = ShaderProgram::create(vs, gs, fs, debug);
        res_[{name, feat}] = shader;
    }
}

std::shared_ptr<ShaderProgram> ShaderManager::get(const std::string &name)
{
    ShaderFeatures none;
    none.reset();
    return get(name, none);
}

std::shared_ptr<ShaderProgram> ShaderManager::get(const std::string &name,
                                                  const ShaderFeatures &features)
{
    return res_.at({name, features});
}

void ShaderManager::clear()
{
    res_.clear();
}

} // namespace rcube