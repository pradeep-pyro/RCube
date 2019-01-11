#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <vector>
#include <string>
#include <memory>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_integer.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace rcube {

class ShaderProgram {
public:
    ShaderProgram();
    ShaderProgram(const ShaderProgram &other) = delete;
    ~ShaderProgram();
    void release();
    static std::shared_ptr<ShaderProgram> create(const std::string &vertex_shader,
                                                 const std::string &fragment_shader,
                                                 bool debug=false);
    static std::shared_ptr<ShaderProgram> create(const std::string &vertex_shader,
                                                 const std::string &geometry_shader,
                                                 const std::string &fragment_shader,
                                                 bool debug=false);
    static std::shared_ptr<ShaderProgram> createFromFile(const std::string &vertex_shader,
                                                         const std::string &fragment_shader,
                                                         bool debug=false);
    static std::shared_ptr<ShaderProgram> createFromFile(const std::string &vertex_shader,
                                                         const std::string &geometry_shader,
                                                         const std::string &fragment_shader,
                                                         bool debug=false);
    void drawArrays(GLint mode, uint32_t first, uint32_t count) const;
    void drawElements(GLint mode, uint32_t first, uint32_t count) const;
    bool link(bool debug=false);
    void use() const;
    void done() const;
    GLint attributeLocation(const std::string &name) const;
    GLint uniformLocation(const std::string &name) const;
    void setUniform(const std::string &name, float val);
    void setUniform(const std::string &name, const glm::vec2 &vec);
    void setUniform(const std::string &name, const glm::vec3 &vec);
    void setUniform(const std::string &name, const glm::vec4 &vec);
    void setUniform(const std::string &name, int val);
    void setUniform(const std::string &name, bool val);
    void setUniform(const std::string &name, const glm::ivec2 &vec);
    void setUniform(const std::string &name, const glm::ivec3 &vec);
    void setUniform(const std::string &name, const glm::ivec4 &vec);
    void setUniform(const std::string &name, const glm::mat2 &mat);
    void setUniform(const std::string &name, const glm::mat3 &mat);
    void setUniform(const std::string &name, const glm::mat4 &mat);
    void showWarnings(bool flag);
private:
    void addShader(GLuint type, const std::string &source, bool debug=false);
    void addShaderFromFile(GLuint type, const std::string &filename, bool debug=false);
    GLuint id_;
    std::vector<GLint> shaders_;
    bool warn_;
};

} // namespace rcube

#endif // SHADERPROGRAM_H
