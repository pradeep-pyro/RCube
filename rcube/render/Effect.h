#ifndef EFFECT_H
#define EFFECT_H

#include <memory>
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Framebuffer.h"

class Effect {
public:
    Effect();
    Effect(const Effect &other) = default;
    Effect & operator=(const Effect &other) = default;
    virtual ~Effect() = default;
    void initialize();
    std::shared_ptr<ShaderProgram> shader() const;
    virtual std::string fragmentShader() = 0;
    virtual void setUniforms() = 0;
    virtual void apply() = 0;
    virtual void resize(int width, int height);
    std::unique_ptr<Framebuffer> result;
protected:
    void renderQuad();
    std::shared_ptr<ShaderProgram> shader_;
    Mesh quad_;
    bool init_;
};

#endif // EFFECT_H
