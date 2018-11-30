#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include "ShaderProgram.h"
#include "Texture.h"

enum RenderPriority {
    Opaque = 0,
    Background = 10,
    Transparent = 20,
    Overlay = 30
};

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

    bool depth_test, depth_mask;
protected:

    struct GLState {
        GLboolean depthmask, depthtest;
    };
    GLState prev_state_;
    std::shared_ptr<ShaderProgram> shader_;

    bool init_;
};

#endif // MATERIAL_H
