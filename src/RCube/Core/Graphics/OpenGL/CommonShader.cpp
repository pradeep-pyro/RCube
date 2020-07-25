#include "RCube/Core/Graphics/OpenGL/CommonShader.h"

namespace rcube
{
namespace common
{

std::shared_ptr<ShaderProgram> skyboxShader()
{
    return ShaderProgram::create(SKYBOX_VERTEX_SHADER, SKYBOX_FRAGMENT_SHADER, true);
}

std::shared_ptr<ShaderProgram> fullScreenQuadShader(const std::string &fragment_shader)
{
    return ShaderProgram::create(FULLSCREEN_QUAD_VERTEX_SHADER, fragment_shader, true);
}

} // namespace common
} // namespace rcube
