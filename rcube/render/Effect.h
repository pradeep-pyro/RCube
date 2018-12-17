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
    std::shared_ptr<Framebuffer> result;
    virtual void use();
    virtual void done();
protected:
    std::shared_ptr<ShaderProgram> shader_;
    bool init_;
};

#endif // EFFECT_H
