#pragma once

#include "RCube/Core/Graphics/OpenGL/GLDataType.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_integer.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace rcube
{

struct ShaderAttributeDesc
{
    ShaderAttributeDesc() = default;
    ShaderAttributeDesc(std::string attribute_name, GLDataType data_type, int array_count = 1)
        : name(attribute_name), type(data_type), count(array_count)
    {
    }
    std::string name;
    GLDataType type;
    int count;
};

class Uniform
{
    std::string name_;
    GLDataType type_;
    int location_;
    GLuint program_id_;

  public:
    Uniform() = default;
    Uniform(std::string name, GLDataType type, int program_id)
        : name_(name), type_(type), program_id_(program_id)
    {
        location_ = glGetUniformLocation(program_id_, name_.c_str());
    }
    std::string name() const
    {
        return name_;
    }
    GLDataType type() const
    {
        return type_;
    }
    void get(bool &val);
    void get(unsigned int &val);
    void get(int &val);
    void get(float &val);
    void get(glm::vec2 &val);
    void get(glm::vec3 &val);
    void get(glm::vec4 &val);
    void get(glm::ivec2 &val);
    void get(glm::ivec3 &val);
    void get(glm::ivec4 &val);
    void set(bool val);
    void set(int val);
    void set(unsigned int val);
    void set(float val);
    void set(glm::mat2 val);
    void set(glm::mat3 val);
    void set(glm::mat4 val);
    void set(glm::vec2 val);
    void set(glm::vec3 val);
    void set(glm::vec4 val);
    void set(glm::ivec2 val);
    void set(glm::ivec3 val);
    void set(glm::ivec4 val);
};

enum RenderPriority
{
    Opaque = 0,
    Background = 10,
    Transparent = 20,
    Overlay = 30
};

enum class BlendFunc
{
    SrcAlpha = GL_SRC_ALPHA,
    OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
    Zero = GL_ZERO,
    One = GL_ONE,
    SrcColor = GL_SRC_COLOR,
    OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
    DstColor = GL_DST_COLOR,
    OneMinusDstColor = GL_ONE_MINUS_DST_COLOR
};

enum class BlendEq
{
    Add = GL_FUNC_ADD,
    Subtract = GL_FUNC_SUBTRACT,
    ReverseSubtract = GL_FUNC_REVERSE_SUBTRACT,
    Max = GL_MAX,
    Min = GL_MIN,
};

enum class Cull
{
    Back = GL_BACK,
    Front = GL_FRONT,
    Both = GL_FRONT_AND_BACK
};

enum class DepthFunc
{
    Less = GL_LESS,   /// Passes if the incoming depth value is less than the stored depth value.
    Equal = GL_EQUAL, /// Passes if the incoming depth value is equal to the stored depth value.
    LessOrEqual = GL_LEQUAL, /// Passes if the incoming depth value is less than or equal to the
                             /// stored depth value.
    Greater =
        GL_GREATER, /// Passes if the incoming depth value is greater than the stored depth value.
    NotEqual =
        GL_NOTEQUAL, /// Passes if the incoming depth value is not equal to the stored depth value.
    GreaterOrEqual = GL_GEQUAL, /// Passes if the incoming depth value is greater than or equal to
                                /// the stored depth value.
    Never = GL_NEVER,           /// Never passes
    Always = GL_ALWAYS,         /// Always passes
};

enum class StencilFunc
{
    Less = GL_LESS,   /// Passes if the incoming depth value is less than the stored depth value.
    Equal = GL_EQUAL, /// Passes if the incoming depth value is equal to the stored depth value.
    LessOrEqual = GL_LEQUAL, /// Passes if the incoming depth value is less than or equal to the
                             /// stored depth value.
    Greater =
        GL_GREATER, /// Passes if the incoming depth value is greater than the stored depth value.
    NotEqual =
        GL_NOTEQUAL, /// Passes if the incoming depth value is not equal to the stored depth value.
    GreaterOrEqual = GL_GEQUAL, /// Passes if the incoming depth value is greater than or equal to
                                /// the stored depth value.
    Never = GL_NEVER,           /// Never passes
    Always = GL_ALWAYS,         /// Always passes
};

enum class StencilOp
{
    Keep = GL_KEEP,
    Zero = GL_ZERO,
    Replace = GL_REPLACE,
    Incr = GL_INCR,
    IncrWrap = GL_INCR_WRAP,
    Decr = GL_DECR,
    DecrWrap = GL_DECR_WRAP,
    Invert = GL_INVERT,
};

enum class PolygonMode : GLenum
{
    Fill = GL_FILL,
    Point = GL_POINT,
    Line = GL_LINE
};

struct RenderSettings
{
    struct Culling
    {
        bool enabled = false;
        Cull mode = Cull::Back;
    };

    struct Depth
    {
        bool test = true;
        bool write = true;
        DepthFunc func = DepthFunc::Less;
        double znear = 0.0;
        double zfar = 1.0;
    };

    struct Stencil
    {
        bool test = false;
        GLuint write = 0xFF;
        StencilFunc func = StencilFunc::Always;
        StencilOp op_stencil_fail = StencilOp::Keep, op_depth_fail = StencilOp::Keep,
                  op_stencil_pass = StencilOp::Keep;
        GLint func_ref = 0;
        GLint func_mask = 0xFF;
    };

    struct Blend
    {
        bool enabled = false;
        struct Blendi
        {
            BlendFunc color_src = BlendFunc::SrcAlpha, color_dst = BlendFunc::OneMinusSrcAlpha;
            BlendFunc alpha_src = BlendFunc::One, alpha_dst = BlendFunc::Zero;
        };
        std::vector<Blendi> blend = {Blendi{}};
        BlendEq equation = BlendEq::Add;
    };

    struct PolygonOffset
    {
        bool enabled = false;
        float offset = 1.f;
    };

    Culling cull;
    Depth depth;
    Stencil stencil;
    Blend blend;
    bool dither = false;
    PolygonMode polygon_mode = PolygonMode::Fill;
    PolygonOffset polygon_offset;
    float line_width = 1.f;
};

class ShaderProgram
{
  private:
    GLuint location_;
    std::vector<GLint> shaders_;
    bool warn_ = true;
    std::unordered_map<std::string, ShaderAttributeDesc> attributes_;
    std::unordered_map<std::string, Uniform> uniforms_;

  public:
    ShaderProgram();
    ShaderProgram(const ShaderProgram &other) = delete;
    ~ShaderProgram();
    void release();
    static std::shared_ptr<ShaderProgram> create(const std::string &vertex_shader,
                                                 const std::string &fragment_shader,
                                                 bool debug = false);
    static std::shared_ptr<ShaderProgram> create(const std::vector<std::string> &vertex_shader,
                                                 const std::vector<std::string> &fragment_shader,
                                                 bool debug = false);
    static std::shared_ptr<ShaderProgram> create(const std::string &vertex_shader,
                                                 const std::string &geometry_shader,
                                                 const std::string &fragment_shader,
                                                 bool debug = false);
    static std::shared_ptr<ShaderProgram> create(const std::vector<std::string> &vertex_shader,
                                                 const std::vector<std::string> &geometry_shader,
                                                 const std::vector<std::string> &fragment_shader,
                                                 bool debug = false);
    const std::unordered_map<std::string, ShaderAttributeDesc> &attributes() const;
    const Uniform &uniform(std::string name) const;
    Uniform &uniform(std::string name);
    bool hasUniform(std::string name, Uniform &uni);
    bool link(bool debug = false);
    GLuint id() const;
    void use() const;
    void done() const;
    GLint attributeLocation(const std::string &name) const;
    GLint uniformLocation(const std::string &name) const;
    void showWarnings(bool flag);

  private:
    void addShader(GLuint type, const std::string &source, bool debug = false);
    void addShader(GLuint type, const std::vector<std::string> &source, bool debug);
    void addShaderFromFile(GLuint type, const std::string &filename, bool debug = false);
    void generateAttributes();
    void generateUniforms();
};

} // namespace rcube
