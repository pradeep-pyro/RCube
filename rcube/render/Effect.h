#ifndef EFFECT_H
#define EFFECT_H

#include <memory>
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Framebuffer.h"

/**
 * Effect is the base class for all image based post processing shaders
 * To implement a new effect, derive from Effect and implement the fragmentShader()
 * and setUniforms() method.
 */
class Effect {
public:
    Effect();
    Effect(const Effect &other) = default;
    Effect & operator=(const Effect &other) = default;
    virtual ~Effect() = default;
    void initialize();
    /**
     * Returns the underlying OpenGL shader program
     * @return Pointer to shader program
     */
    std::shared_ptr<ShaderProgram> shader() const;
    /**
     * Fragment shader of the effect.
     * This method has to be implemented in the derived class.
     * @return Fragment shader source string
     */
    virtual std::string fragmentShader() = 0;
    /**
     * Set uniforms that are used in the fragment shader
     */
    virtual void setUniforms() = 0;
    /**
     * Use the effect: Binds shaders, set uniforms etc.
     */
    virtual void use();
    /**
     * Cleanup OpenGL state related to this effect.
     */
    virtual void done();

    std::shared_ptr<Framebuffer> result;  /// Framebuffer to store result obtained after applying this effect
protected:
    std::shared_ptr<ShaderProgram> shader_;
    bool init_;
};

#endif // EFFECT_H
