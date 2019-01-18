#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <memory>
#include "Image.h"

namespace rcube {

enum class TextureWrapMode {
    Repeat = GL_REPEAT,
    MirroredRepeat = GL_MIRRORED_REPEAT,
    ClampToEdge = GL_CLAMP_TO_EDGE,
    ClampToBorder = GL_CLAMP_TO_BORDER
};

enum class TextureFilterMode {
    Nearest = GL_NEAREST,
    Linear = GL_LINEAR
};

enum class TextureFormat {
    Red = GL_RED,
    RG = GL_RG,
    RGB = GL_RGB,
    BGR = GL_BGR,
    RGBA = GL_RGBA,
    BGRA = GL_BGRA,
    Depth = GL_DEPTH_COMPONENT,
    DepthStencil = GL_DEPTH_STENCIL,
};

enum class TextureInternalFormat {
    R8 = GL_R8,
    R16 = GL_R16,
    R16F = GL_R16F,
    R32F = GL_R32F,
    RG8 = GL_RG8,
    RG16 = GL_RG16,
    RG16F = GL_RG16F,
    RG32F = GL_RG32F,
    RGB8 = GL_RGB8,
    RGB16 = GL_RGB16,
    RGB16F = GL_RGB16F,
    RGB32F = GL_RGB32F,
    RGBA8 = GL_RGBA8,
    RGBA16 = GL_RGBA16,
    RGBA16F = GL_RGBA16F,
    RGBA32F = GL_RGBA32F,
    Depth16 = GL_DEPTH_COMPONENT16,
    Depth24 = GL_DEPTH_COMPONENT24,
    Depth32 = GL_DEPTH_COMPONENT32,
    Depth32F = GL_DEPTH_COMPONENT32F,
    Depth24Stencil8 = GL_DEPTH24_STENCIL8,
    Depth32FStencil8 = GL_DEPTH32F_STENCIL8,
};

class Texture2D {
public:
    Texture2D() = default;
    ~Texture2D();
    static std::shared_ptr<Texture2D> create(size_t width, size_t height, size_t levels,
                                             TextureInternalFormat internal_format=TextureInternalFormat::RGBA8,
                                             size_t samples=0);
    void release();
    void setWrapMode(TextureWrapMode mode);
    void setWrapModeS(TextureWrapMode wrap_s);
    void setWrapModeT(TextureWrapMode wrap_t);
    void setData(const float *data, TextureFormat format, size_t level=0);
    void setData(const unsigned char *data, TextureFormat format, size_t level=0);
    void setData(const Image &im);
    size_t width() const;
    size_t height() const;
    size_t levels() const;
    GLenum target() const;
    TextureInternalFormat internalFormat() const;
    GLuint id() const;
    void use(size_t unit=0);
    void done();
    size_t numSamples() const;
    void setBorderColor(const glm::vec4 &color);
    void setFilterModeMin(TextureFilterMode mode);
    void setFilterModeMag(TextureFilterMode mode);
    void setFilterMode(TextureFilterMode mode);
    void generateMipMap();
    bool valid() const;
private:
    GLuint id_ = 0;
    size_t unit_ = 0;
    bool in_use_ = false;
    size_t width_, height_, levels_;
    size_t num_samples_ = 0;
    GLenum target_;
    TextureInternalFormat internal_format_;
};

class TextureCubemap {
public:
    enum Side {
        PositiveX = 0,
        NegativeX = 1,
        PositiveY = 2,
        NegativeY = 3,
        PositiveZ = 4,
        NegativeZ = 5,
    };
    TextureCubemap() = default;
    ~TextureCubemap();
    static std::shared_ptr<TextureCubemap> create(size_t width, size_t height, size_t levels=1, bool seamless=true,
                                                  TextureInternalFormat internal_format=TextureInternalFormat::RGBA8);
    void release();
    void setWrapMode(TextureWrapMode mode);
    void setWrapModeS(TextureWrapMode wrap_s);
    void setWrapModeT(TextureWrapMode wrap_t);
    void setWrapModeR(TextureWrapMode mode);
    void setWrapMode(TextureWrapMode s, TextureWrapMode t);
    void setData(int i, const unsigned char *data, size_t width, size_t height, size_t level, TextureFormat format);
    void setData(int i, const Image &im, size_t level=0);
    size_t width() const;
    size_t height() const;
    size_t levels() const;
    TextureInternalFormat internalFormat() const;
    GLuint id() const;
    void use(size_t unit=0);
    void done();
    void setFilterModeMin(TextureFilterMode mode);
    void setFilterModeMag(TextureFilterMode mode);
    void setFilterMode(TextureFilterMode mode);
    void generateMipMap();
    bool valid() const;
private:
    GLuint id_ = 0;
    size_t unit_ = 0;
    bool in_use_ = false;
    size_t width_, height_, levels_;
    bool seamless_ = true;
    TextureInternalFormat internal_format_;
};

} // namespace rcube

#endif // TEXTURE_H
