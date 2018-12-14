#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include "ShaderProgram.h"
#include "Texture.h"
#include "RenderSettings.h"

class Material {
public:
    Material();
    Material(const Material &other) = default;
    Material & operator=(const Material &other) = default;
    virtual ~Material();
    void initialize();
    std::shared_ptr<ShaderProgram> shader() const;
    virtual std::string vertexShader() = 0;
    virtual std::string fragmentShader() = 0;
    virtual std::string geometryShader() = 0;
    virtual void setUniforms() = 0;
    virtual int renderPriority() const = 0;
    virtual void use();
    virtual void done();

    RenderSettings render_settings;

protected:
    std::shared_ptr<ShaderProgram> shader_;
    bool init_;
};

#endif // MATERIAL_H
