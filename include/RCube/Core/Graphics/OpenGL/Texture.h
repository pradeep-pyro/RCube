#ifndef TEXTURE_H
#define TEXTURE_H

#include "RCube/Core/Graphics/OpenGL/Image.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <memory>
#include <string>

namespace rcube
{

enum class TextureWrapMode
{
    Repeat = GL_REPEAT,
    MirroredRepeat = GL_MIRRORED_REPEAT,
    ClampToEdge = GL_CLAMP_TO_EDGE,
    ClampToBorder = GL_CLAMP_TO_BORDER
};

enum class TextureFilterMode
{
    Nearest = GL_NEAREST,
    Linear = GL_LINEAR,
    Trilinear = GL_LINEAR_MIPMAP_LINEAR
};

enum class TextureFormat
{
    Red = GL_RED,
    RG = GL_RG,
    RGB = GL_RGB,
    BGR = GL_BGR,
    RGBA = GL_RGBA,
    BGRA = GL_BGRA,
    Depth = GL_DEPTH_COMPONENT,
    DepthStencil = GL_DEPTH_STENCIL,
};

enum class TextureInternalFormat : GLenum
{
    R8 = GL_R8,
    R16 = GL_R16,
    R16F = GL_R16F,
    R16I = GL_R16I,
    R32F = GL_R32F,
    R32I = GL_R32I,
    R32UI = GL_R32UI,
    RG8 = GL_RG8,
    RG16 = GL_RG16,
    RG32I = GL_RG32I,
    RG32UI = GL_RG32UI,
    RG16F = GL_RG16F,
    RG32F = GL_RG32F,
    RGB8 = GL_RGB8,
    RGB16 = GL_RGB16,
    RGB16F = GL_RGB16F,
    RGB16I = GL_RGB16I,
    RGB32F = GL_RGB32F,
    RGB32I = GL_RGB32I,
    RGB32UI = GL_RGB32UI,
    sRGB8 = GL_SRGB8,
    RGBA8 = GL_RGBA8,
    sRGBA8 = GL_SRGB8_ALPHA8,
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

class Texture2D
{
    GLenum internalFormatToFormat() const;
  public:
    Texture2D() = default;
    ~Texture2D();
    Texture2D(const Texture2D &other) = delete;
    static std::shared_ptr<Texture2D>
    create(size_t width, size_t height, size_t levels,
           TextureInternalFormat internal_format = TextureInternalFormat::RGBA8);
    static std::shared_ptr<Texture2D>
    createMS(size_t width, size_t height, size_t num_samples,
             TextureInternalFormat internal_format = TextureInternalFormat::RGBA8);
    void release();
    void setWrapMode(TextureWrapMode mode);
    void setWrapModeS(TextureWrapMode wrap_s);
    void setWrapModeT(TextureWrapMode wrap_t);
    void setData(const float *data, TextureFormat format, size_t level = 0);
    void setData(const unsigned char *data, TextureFormat format, size_t level = 0);
    void setData(const Image &im);
    void setData(const Image &im, TextureFormat fmt);
    size_t width() const;
    size_t height() const;
    size_t levels() const;
    GLenum target() const;
    TextureInternalFormat internalFormat() const;
    GLuint id() const;
    void use(size_t unit = 0);
    void done();
    size_t numSamples() const;
    void setBorderColor(const glm::vec4 &color);
    void setFilterModeMin(TextureFilterMode mode);
    void setFilterModeMag(TextureFilterMode mode);
    void setFilterMode(TextureFilterMode mode);
    void generateMipMap();
    bool valid() const;
    void getSubImage(int x, int y, int width, int height, int *pixels, size_t size) const;
    void getSubImage(int x, int y, int width, int height, uint32_t *pixels, size_t size) const;
    void getSubImage(int x, int y, int width, int height, float *pixels, size_t size) const;

  private:
    GLuint id_ = 0;
    size_t unit_ = 0;
    bool in_use_ = false;
    size_t width_, height_, levels_;
    size_t num_samples_ = 0;
    GLenum target_;
    TextureInternalFormat internal_format_;
};

class TextureCubemap
{
  public:
    enum Side
    {
        PositiveX = 0,
        NegativeX = 1,
        PositiveY = 2,
        NegativeY = 3,
        PositiveZ = 4,
        NegativeZ = 5,
    };
    TextureCubemap() = default;
    ~TextureCubemap();
    static std::shared_ptr<TextureCubemap>
    create(size_t width, size_t height, size_t levels = 1, bool seamless = true,
           TextureInternalFormat internal_format = TextureInternalFormat::RGBA8);
    void release();
    void setWrapMode(TextureWrapMode mode);
    void setWrapModeS(TextureWrapMode wrap_s);
    void setWrapModeT(TextureWrapMode wrap_t);
    void setWrapModeR(TextureWrapMode mode);
    void setData(Side i, const unsigned char *data, size_t width, size_t height, size_t level,
                 TextureFormat format);
    void setData(Side i, const Image &im, size_t level = 0);
    size_t width() const;
    size_t height() const;
    size_t levels() const;
    TextureInternalFormat internalFormat() const;
    GLuint id() const;
    void use(size_t unit = 0);
    void done();
    void setFilterModeMin(TextureFilterMode mode);
    void setFilterModeMag(TextureFilterMode mode);
    void setFilterMode(TextureFilterMode mode);
    void generateMipMap();
    bool valid() const;
    GLenum target() const;
    GLenum target(Side side) const;

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
