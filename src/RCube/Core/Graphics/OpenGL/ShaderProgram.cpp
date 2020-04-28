#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace rcube
{

std::string getStringFromFile(const std::string &filename)
{
    std::ifstream f(filename);
    if (!f.is_open())
    {
        throw std::runtime_error("Unable to open shader source file: " + filename);
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

ShaderProgram::ShaderProgram() : location_(0), warn_(false)
{
}

ShaderProgram::~ShaderProgram()
{
    release();
}

std::shared_ptr<ShaderProgram> ShaderProgram::create(const VertexShader &vertex_shader,
                                                     const FragmentShader &fragment_shader,
                                                     bool debug)
{
    auto prog = std::make_shared<ShaderProgram>();
    prog->addShader(GL_VERTEX_SHADER, vertex_shader.source, debug);
    prog->addShader(GL_FRAGMENT_SHADER, fragment_shader.source, debug);
    prog->link(debug);

    // Get all attributes
    for (const ShaderAttributeDesc &attr : vertex_shader.attributes)
    {
        prog->attributes_.push_back(attr);
    }

    // Get all uniforms
    prog->available_uniforms_.reserve(vertex_shader.uniforms.size() +
                                      fragment_shader.uniforms.size());
    prog->available_uniforms_.insert(prog->available_uniforms_.end(),
                                     vertex_shader.uniforms.begin(), vertex_shader.uniforms.end());
    prog->available_uniforms_.insert(prog->available_uniforms_.end(),
                                     fragment_shader.uniforms.begin(),
                                     fragment_shader.uniforms.end());
    for (const ShaderUniformDesc &uniform_desc : vertex_shader.uniforms)
    {
        GLint id = prog->uniformLocation(uniform_desc.name);
        prog->uniforms_[uniform_desc.name] =
            Uniform(uniform_desc.name, uniform_desc.type, prog->location_);
    }
    for (const ShaderUniformDesc &uniform_desc : fragment_shader.uniforms)
    {
        GLint id = prog->uniformLocation(uniform_desc.name);
        prog->uniforms_[uniform_desc.name] =
            Uniform(uniform_desc.name, uniform_desc.type, prog->location_);
    }

    // Get all textures
    for (const ShaderTextureDesc &texture : fragment_shader.textures)
    {
        if (texture.dim == 2)
        {
            glGetUniformiv(prog->location_,
                           glGetUniformLocation(prog->location_, texture.name.c_str()),
                           &(prog->textures_[texture.name].unit));
            prog->textures_[texture.name].texture = nullptr;
        }
    }
    for (const ShaderCubemapDesc &cubemap : fragment_shader.cubemaps)
    {
        glGetUniformiv(prog->location_, glGetUniformLocation(prog->location_, cubemap.name.c_str()),
                       &(prog->cubemaps_[cubemap.name].unit));
        prog->cubemaps_[cubemap.name].cubemap = nullptr;
    }

    return prog;
}

std::shared_ptr<ShaderProgram> ShaderProgram::create(const VertexShader &vertex_shader,
                                                     const GeometryShader &geometry_shader,
                                                     const FragmentShader &fragment_shader,
                                                     bool debug)
{
    auto prog = std::make_shared<ShaderProgram>();
    prog->addShader(GL_VERTEX_SHADER, vertex_shader.source, debug);
    prog->addShader(GL_GEOMETRY_SHADER, geometry_shader.source, debug);
    prog->addShader(GL_FRAGMENT_SHADER, fragment_shader.source, debug);
    prog->link();

    // Get all attributes
    for (const ShaderAttributeDesc &attr : vertex_shader.attributes)
    {
        prog->attributes_.push_back(attr);
    }
    for (const ShaderAttributeDesc &attr : geometry_shader.attributes)
    {
        prog->attributes_.push_back(attr);
    }

    // Get all uniforms
    prog->available_uniforms_.reserve(vertex_shader.uniforms.size() +
                                      geometry_shader.uniforms.size() +
                                      fragment_shader.uniforms.size());
    prog->available_uniforms_.insert(prog->available_uniforms_.end(),
                                     vertex_shader.uniforms.begin(), vertex_shader.uniforms.end());
    prog->available_uniforms_.insert(prog->available_uniforms_.end(),
                                     geometry_shader.uniforms.begin(),
                                     geometry_shader.uniforms.end());
    prog->available_uniforms_.insert(prog->available_uniforms_.end(),
                                     fragment_shader.uniforms.begin(),
                                     fragment_shader.uniforms.end());
    for (const ShaderUniformDesc &uniform_desc : vertex_shader.uniforms)
    {
        GLint id = prog->uniformLocation(uniform_desc.name);
        prog->uniforms_[uniform_desc.name] =
            Uniform(uniform_desc.name, uniform_desc.type, prog->location_);
    }
    for (ShaderUniformDesc uniform_desc : geometry_shader.uniforms)
    {
        GLint id = prog->uniformLocation(uniform_desc.name);
        prog->uniforms_[uniform_desc.name] =
            Uniform(uniform_desc.name, uniform_desc.type, prog->location_);
    }
    for (const ShaderUniformDesc &uniform_desc : fragment_shader.uniforms)
    {
        GLint id = prog->uniformLocation(uniform_desc.name);
        prog->uniforms_[uniform_desc.name] =
            Uniform(uniform_desc.name, uniform_desc.type, prog->location_);
    }

    // Get all textures
    for (const ShaderTextureDesc &texture : fragment_shader.textures)
    {
        if (texture.dim == 2)
        {
            glGetUniformiv(prog->location_,
                           glGetUniformLocation(prog->location_, texture.name.c_str()),
                           &(prog->textures_[texture.name].unit));
            prog->textures_[texture.name].texture = nullptr;
        }
    }
    for (const ShaderCubemapDesc &cubemap : fragment_shader.cubemaps)
    {
        glGetUniformiv(prog->location_, glGetUniformLocation(prog->location_, cubemap.name.c_str()),
                       &(prog->cubemaps_[cubemap.name].unit));
        prog->cubemaps_[cubemap.name].cubemap = nullptr;
    }

    return prog;
}

void ShaderProgram::release()
{
    if (location_ != 0)
    {
        glDeleteProgram(location_);
        location_ = 0;
    }
}

// Links the shader program and if successful detaches all associated shaders
bool ShaderProgram::link(bool debug)
{
    glLinkProgram(location_);
    GLint success = 0;
    glGetProgramiv(location_, GL_LINK_STATUS, (int *)&success);
    if (!success)
    {
        if (debug)
        {
            // Print out the error log in case of unsuccessful linking
            char log[512];
            glGetProgramInfoLog(location_, 512, nullptr, log);
            std::cout << "Linking error in shader program:\n" << log << std::endl;
        }
        location_ = 0;
        return false;
    }
    // Detach shaders after successful linking
    for (GLuint shader : shaders_)
    {
        glDetachShader(location_, shader);
    }
    return true;
}

GLuint ShaderProgram::id() const
{
    return location_;
}

// Use the shader (glUseProgram(my id))
void ShaderProgram::use() const
{
    glUseProgram(location_);
    for (auto &keyval : textures_)
    {
        if (keyval.second.texture != nullptr)
        {
            keyval.second.texture->use(keyval.second.unit);
            // glUniform1i(keyval.second.sampler, texture_unit);
        }
    }
    for (auto &keyval : cubemaps_)
    {
        if (keyval.second.cubemap != nullptr)
        {
            volatile GLint unittest = keyval.second.unit;
            keyval.second.cubemap->use(keyval.second.unit);
            // glUniform1i(keyval.second.sampler, texture_unit);
        }
    }
}

// Done with the shader (glUseProgram(0))
void ShaderProgram::done() const
{
    glUseProgram(0);
}

// Get the location of the given attribute
GLint ShaderProgram::attributeLocation(const std::string &name) const
{
    GLint id = glGetAttribLocation(location_, name.c_str());
    return id;
}

// Get the location of the given uniform
GLint ShaderProgram::uniformLocation(const std::string &name) const
{
    return glGetUniformLocation(location_, name.c_str());
}

void ShaderProgram::showWarnings(bool flag)
{
    warn_ = flag;
}

const std::vector<ShaderUniformDesc> &ShaderProgram::availableUniforms() const
{
    return available_uniforms_;
}

RenderSettings &ShaderProgram::renderState()
{
    return render_state_;
}

RenderPriority &ShaderProgram::renderPriority()
{
    return render_priority_;
}

void ShaderProgram::addShader(GLuint type, const std::string &source, bool debug)
{
    // Create a new program if not already done
    if (location_ == 0)
    {
        location_ = glCreateProgram();
    }
    GLuint shader = glCreateShader(type);
    const char *c_str = source.c_str();
    GLint length = source.size();
    glShaderSource(shader, 1, &c_str, &length);
    // Try to compile the shader
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // Shader compilation failed; print the log if debug==true
        if (debug)
        {
            char log[512];
            int log_size;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::string str_type;
            if (type == GL_VERTEX_SHADER)
            {
                str_type = "vertex";
            }
            else if (type == GL_FRAGMENT_SHADER)
            {
                str_type = "fragment";
            }
            else if (type == GL_GEOMETRY_SHADER)
            {
                str_type = "geometry";
            }
            std::cerr << "Compilation error in " << str_type << " shader:\n" << log << std::endl;
        }
        throw std::runtime_error("Unable to compile shader");
    }
    // Shader compilation successful; attach the shader to the program
    glAttachShader(location_, shader);
    shaders_.push_back(shader);
}

void ShaderProgram::addShaderFromFile(GLuint type, const std::string &filename, bool debug)
{
    std::string src = getStringFromFile(filename);
    addShader(type, src, debug);
}

const std::vector<ShaderAttributeDesc> &ShaderProgram::attributes() const
{
    return attributes_;
}
bool ShaderProgram::hasUniform(std::string name, Uniform &uni)
{
    auto it = uniforms_.find(name);
    if (it == uniforms_.end())
    {
        return false;
    }
    uni = it->second;
    return true;
}
const Uniform &ShaderProgram::uniform(
    std::string name) const
{
    return uniforms_.at(name);
}
Uniform &ShaderProgram::uniform(std::string name)
{
    return uniforms_.at(name);
}
const std::shared_ptr<Texture2D> &ShaderProgram::texture(std::string name) const
{
    return textures_.at(name).texture;
}
std::shared_ptr<Texture2D> &ShaderProgram::texture(std::string name)
{
    return textures_.at(name).texture;
}
const std::shared_ptr<TextureCubemap> &ShaderProgram::cubemap(std::string name) const
{
    return cubemaps_.at(name).cubemap;
}
std::shared_ptr<TextureCubemap> &ShaderProgram::cubemap(std::string name)
{
    return cubemaps_.at(name).cubemap;
}

///////////////////////////////////////////////////////////////////////////////

void Uniform::set(bool val)
{
    assert(type_ == GLDataType::Bool && "Invalid type for uniform");
    glProgramUniform1i(program_id_, location_, static_cast<int>(val));
}

void Uniform::set(int val)
{
    assert(type_ == GLDataType::Int && "Invalid type for uniform");
    glProgramUniform1i(program_id_, location_, val);
}

void Uniform::set(unsigned int val)
{
    assert(type_ == GLDataType::Uint && "Invalid type for uniform");
    glProgramUniform1ui(program_id_, location_, val);
}

void Uniform::set(float val)
{
    assert(type_ == GLDataType::Float && "Invalid type for uniform");
    glProgramUniform1f(program_id_, location_, val);
}

void Uniform::set(glm::mat2 val)
{
    assert(type_ == GLDataType::Mat2f && "Invalid type for uniform");
    glProgramUniformMatrix2fv(program_id_, location_, 1, GL_FALSE, glm::value_ptr(val));
}

void Uniform::set(glm::mat3 val)
{
    assert(type_ == GLDataType::Mat3f && "Invalid type for uniform");
    glProgramUniformMatrix3fv(program_id_, location_, 1, GL_FALSE, glm::value_ptr(val));
}

void Uniform::set(glm::mat4 val)
{
    assert(type_ == GLDataType::Mat4f && "Invalid type for uniform");
    glProgramUniformMatrix4fv(program_id_, location_, 1, GL_FALSE, glm::value_ptr(val));
}

void Uniform::set(glm::vec2 val)
{
    assert(type_ == GLDataType::Vec2f && "Invalid type for uniform");
    glProgramUniform2f(program_id_, location_, val[0], val[1]);
}

void Uniform::set(glm::vec3 val)
{
    assert(type_ == GLDataType::Vec3f && "Invalid type for uniform");
    glProgramUniform3f(program_id_, location_, val[0], val[1], val[2]);
}

void Uniform::set(glm::vec4 val)
{
    assert(type_ == GLDataType::Vec4f && "Invalid type for uniform");
    glProgramUniform4f(program_id_, location_, val[0], val[1], val[2], val[3]);
}

void Uniform::set(glm::ivec2 val)
{
    assert(type_ == GLDataType::Vec2i && "Invalid type for uniform");
    glProgramUniform2i(program_id_, location_, val[0], val[1]);
}

void Uniform::set(glm::ivec3 val)
{
    assert(type_ == GLDataType::Vec3i && "Invalid type for uniform");
    glProgramUniform3i(program_id_, location_, val[0], val[1], val[2]);
}

void Uniform::set(glm::ivec4 val)
{
    assert(type_ == GLDataType::Vec4i && "Invalid type for uniform");
    glProgramUniform4i(program_id_, location_, val[0], val[1], val[2], val[3]);
}

void Uniform::get(bool &val)
{
    GLint ret;
    glGetUniformiv(program_id_, location_, &ret);
    val = static_cast<bool>(ret);
}

void Uniform::get(int &val)
{
    GLint ret;
    glGetUniformiv(program_id_, location_, &ret);
    val = static_cast<int>(ret);
}

void Uniform::get(unsigned int &val)
{
    GLuint ret;
    glGetUniformuiv(program_id_, location_, &ret);
    val = static_cast<unsigned int>(ret);
}

void Uniform::get(float &val)
{
    GLfloat ret;
    glGetUniformfv(program_id_, location_, &ret);
    val = static_cast<float>(ret);
}

void Uniform::get(glm::vec2 &val)
{
    glGetUniformfv(program_id_, location_, glm::value_ptr(val));
}

void Uniform::get(glm::ivec2 &val)
{
    glGetUniformiv(program_id_, location_, glm::value_ptr(val));
}

void Uniform::get(glm::vec3 &val)
{
    glGetUniformfv(program_id_, location_, glm::value_ptr(val));
}

void Uniform::get(glm::ivec3 &val)
{
    glGetUniformiv(program_id_, location_, glm::value_ptr(val));
}

void Uniform::get(glm::vec4 &val)
{
    glGetUniformfv(program_id_, location_, glm::value_ptr(val));
}

void Uniform::get(glm::ivec4 &val)
{
    glGetUniformiv(program_id_, location_, glm::value_ptr(val));
}

} // namespace rcube
