#ifndef MATERIAL_H
#define MATERIAL_H

#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"
#include <memory>
namespace rcube
{

/**
 *
 */
enum class Combine
{
    Multiply = 0,
    Add = 1,
    Mix = 2
};

class Material
{
  public:
    Material();
    Material(std::shared_ptr<ShaderProgram> program, RenderSettings state, RenderPriority priority);
    Material(const Material &other) = default;
    Material &operator=(const Material &other) = default;
    virtual ~Material();
    void initialize();
    std::shared_ptr<ShaderProgram> shader() const;
    /*virtual std::string vertexShader() = 0;
    virtual std::string fragmentShader() = 0;
    virtual std::string geometryShader() = 0;*/
    virtual VertexShader vertexShader() = 0;
    virtual FragmentShader fragmentShader() = 0;
    virtual GeometryShader geometryShader() = 0;
    virtual void setUniforms() = 0;
    Uniform &uniform(std::string name);
    virtual int renderPriority() const = 0;
    virtual void use();
    virtual void done();

    RenderSettings render_settings;

  protected:
    std::shared_ptr<ShaderProgram> shader_;
    bool init_;
};

} // namespace rcube

#endif // MATERIAL_H
