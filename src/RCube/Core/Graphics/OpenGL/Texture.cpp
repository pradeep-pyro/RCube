#include "RCube/Core/Graphics/OpenGL/Texture.h"
#include "glm/gtc/type_ptr.hpp"
#include <vector>

namespace rcube
{

const std::string ERROR_IMAGE_TEXTURE_MISMATCH =
    "Image dimensions are different from allocated texture dimensions";
const std::string ERROR_IMAGE_CHANNELS_MISMATCH =
    "Unexpected number of channels, expected 1, 3, or 4";
const std::string ERROR_TEXTURE_UNINITIALIZED = "Cannot use texture without initializing";

// --------------------------------------
// Texture2D
// --------------------------------------

std::shared_ptr<Texture2D> Texture2D::create(size_t width, size_t height, size_t levels,
                                             TextureInternalFormat internal_format)
{
    auto tex = std::make_shared<Texture2D>();
    tex->width_ = width;
    tex->height_ = height;
    tex->levels_ = levels;
    tex->internal_format_ = internal_format;
    tex->num_samples_ = 0;
    tex->target_ = GL_TEXTURE_2D;
    glCreateTextures(GL_TEXTURE_2D, 1, &tex->id_);
    glTextureStorage2D(tex->id_, (GLsizei)levels, (GLenum)internal_format, (GLsizei)width,
                       (GLsizei)height);
    tex->setWrapMode(TextureWrapMode::ClampToEdge);
    tex->setFilterMode(TextureFilterMode::Linear);
    return tex;
}

std::shared_ptr<Texture2D> Texture2D::createMS(size_t width, size_t height, size_t num_samples,
                                               TextureInternalFormat internal_format)
{
    auto tex = std::make_shared<Texture2D>();
    tex->width_ = width;
    tex->height_ = height;
    tex->levels_ = 1;
    tex->internal_format_ = internal_format;
    tex->num_samples_ = num_samples;
    tex->target_ = GL_TEXTURE_2D_MULTISAMPLE;
    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &tex->id_);
    glTextureStorage2DMultisample(tex->id_, (GLsizei)tex->num_samples_, (GLenum)internal_format,
                                  (GLsizei)width, (GLsizei)height, GL_TRUE);
    tex->setWrapMode(TextureWrapMode::ClampToEdge);
    tex->setFilterMode(TextureFilterMode::Linear);
    return tex;
}

Texture2D::~Texture2D()
{
    release();
}

bool Texture2D::valid() const
{
    return id_ > 0;
}

GLenum Texture2D::internalFormatToFormat() const
{
    GLenum format;
    // GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_DEPTH_COMPONENT and GL_STENCIL_INDEX
    switch (internal_format_)
    {
    case TextureInternalFormat::Depth16:
    case TextureInternalFormat::Depth24:
    case TextureInternalFormat::Depth32:
    case TextureInternalFormat::Depth32F:
    {
        format = GL_DEPTH_COMPONENT;
        break;
    }
    case TextureInternalFormat::Depth24Stencil8:
    case TextureInternalFormat::Depth32FStencil8:
    {
        format = GL_DEPTH_STENCIL;
        break;
    }
    case TextureInternalFormat::R8:
    case TextureInternalFormat::R16F:
    case TextureInternalFormat::R32F:
    {
        format = GL_RED;
        break;
    }
    case TextureInternalFormat::R32UI:
    case TextureInternalFormat::R16I:
    case TextureInternalFormat::R32I:
    {
        format = GL_RED_INTEGER;
        break;
    }
    case TextureInternalFormat::RG16F:
    case TextureInternalFormat::RG16:
    case TextureInternalFormat::RG32F:
    case TextureInternalFormat::RG8:
    {
        format = GL_RG;
        break;
    }
    case TextureInternalFormat::RG32UI:
    case TextureInternalFormat::RG32I:
    {
        format = GL_RG_INTEGER;
        break;
    }
    case TextureInternalFormat::RGB16:
    case TextureInternalFormat::RGB16F:
    case TextureInternalFormat::RGB32F:
    case TextureInternalFormat::RGB8:
    case TextureInternalFormat::sRGB8:
    {
        format = GL_RGB;
        break;
    }
    case TextureInternalFormat::RGB16I:
    case TextureInternalFormat::RGB32I:
    case TextureInternalFormat::RGB32UI:
    {
        format = GL_RGB_INTEGER;
        break;
    }
    case TextureInternalFormat::RGBA16:
    case TextureInternalFormat::RGBA16F:
    case TextureInternalFormat::RGBA32F:
    case TextureInternalFormat::RGBA8:
    case TextureInternalFormat::sRGBA8:
    {
        format = GL_RGBA;
        break;
    }
    }
    return format;
}

void Texture2D::getSubImage(int x, int y, int width, int height,
                            int *pixels, size_t size) const
{
    GLenum format = internalFormatToFormat();
    glGetTextureSubImage(id_, 0, (GLint)x, (GLint)y, 0, (GLint)width, (GLint)height, 1,
                         format, GL_INT, GLsizei(size) * sizeof(int),
                         pixels);
}

void Texture2D::getSubImage(int x, int y, int width, int height, uint32_t *pixels, size_t size) const
{
    GLenum format = internalFormatToFormat();
    glGetTextureSubImage(id_, 0, (GLint)x, (GLint)y, 0, (GLint)width, (GLint)height, 1, format,
                         GL_UNSIGNED_INT, GLsizei(size) * sizeof(uint32_t), pixels);
}

void Texture2D::getSubImage(int x, int y, int width, int height, float *pixels, size_t size) const
{
    GLenum format = internalFormatToFormat();
    glGetTextureSubImage(id_, 0, (GLint)x, (GLint)y, 0, (GLint)width, (GLint)height, 1, format,
                         GL_FLOAT, GLsizei(size) * sizeof(float), pixels);
}

void Texture2D::release()
{
    if (valid())
    {
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
}

void Texture2D::setBorderColor(const glm::vec4 &color)
{
    if (num_samples_ > 0)
    {
        return;
    }
    glTextureParameterfv(id_, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
}

void Texture2D::setFilterMode(TextureFilterMode mode)
{
    if (num_samples_ > 0)
    {
        return;
    }
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, (GLint)mode);
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, (GLint)mode);
}

void Texture2D::setFilterModeMin(TextureFilterMode mode)
{
    if (num_samples_ > 0)
    {
        return;
    }
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, (GLint)mode);
}

void Texture2D::setFilterModeMag(TextureFilterMode mode)
{
    if (num_samples_ > 0)
    {
        return;
    }
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, (GLint)mode);
}

void Texture2D::generateMipMap()
{
    if (num_samples_ > 0)
    {
        return;
    }
    glGenerateTextureMipmap(id_);
}

void Texture2D::setWrapMode(TextureWrapMode mode)
{
    if (num_samples_ > 0)
    {
        return;
    }
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, (GLint)mode);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, (GLint)mode);
}

void Texture2D::setWrapModeS(TextureWrapMode mode)
{
    if (num_samples_ > 0)
    {
        return;
    }
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, (GLint)mode);
}

void Texture2D::setWrapModeT(TextureWrapMode mode)
{
    if (num_samples_ > 0)
    {
        return;
    }
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, (GLint)mode);
}

void Texture2D::setData(const unsigned char *data, TextureFormat format, size_t level)
{
    use();
    glTexSubImage2D(target_, (GLint)level, 0, 0, (GLsizei)width_, (GLsizei)height_, (GLenum)format,
                    GL_UNSIGNED_BYTE, data);
    generateMipMap();
    done();
}

void Texture2D::setData(const float *data, TextureFormat format, size_t level)
{
    use();
    glTexSubImage2D(target_, (GLint)level, 0, 0, (GLsizei)width_, (GLsizei)height_, (GLenum)format,
                    GL_FLOAT, data);
    generateMipMap();
    done();
}

void Texture2D::setData(const Image &im)
{
    if (im.width() != width_ || im.height() != height_)
    {
        throw std::invalid_argument(ERROR_IMAGE_TEXTURE_MISMATCH);
    }
    TextureFormat format;
    if (im.channels() == 3)
    {
        format = TextureFormat::RGB;
    }
    else if (im.channels() == 4)
    {
        format = TextureFormat::RGBA;
    }
    else if (im.channels() == 1)
    {
        format = TextureFormat::Red;
    }
    else
    {
        throw std::runtime_error(ERROR_IMAGE_CHANNELS_MISMATCH);
    }
    setData(im.pixels().data(), format);
}

void Texture2D::setData(const Image &im, TextureFormat fmt)
{
    if (im.width() != width_ || im.height() != height_)
    {
        throw std::invalid_argument(ERROR_IMAGE_TEXTURE_MISMATCH);
    }
    setData(im.pixels().data(), fmt);
}

size_t Texture2D::width() const
{
    return width_;
}

size_t Texture2D::height() const
{
    return height_;
}

size_t Texture2D::levels() const
{
    return levels_;
}

size_t Texture2D::numSamples() const
{
    return num_samples_;
}

GLenum Texture2D::target() const
{
    return target_;
}

TextureInternalFormat Texture2D::internalFormat() const
{
    return internal_format_;
}

GLuint Texture2D::id() const
{
    return id_;
}

void Texture2D::use(size_t unit)
{
    if (!valid())
    {
        throw std::runtime_error(ERROR_TEXTURE_UNINITIALIZED);
    }
    unit_ = unit;
    glBindTextureUnit((GLint)unit, id_);
}

void Texture2D::done()
{
    if (in_use_)
    {
        glActiveTexture(GL_TEXTURE0 + (GLenum)unit_);
        glBindTexture(target_, 0);
        in_use_ = false;
    }
}

// --------------------------------------
// TextureCube
// --------------------------------------
TextureCubemap::~TextureCubemap()
{
    release();
}

std::shared_ptr<TextureCubemap> TextureCubemap::create(size_t width, size_t height, size_t levels,
                                                       bool seamless,
                                                       TextureInternalFormat internal_format)
{
    auto tex = std::make_shared<TextureCubemap>();
    tex->width_ = width;
    tex->height_ = height;
    tex->levels_ = levels;
    tex->internal_format_ = internal_format;
    tex->seamless_ = seamless;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &tex->id_);
    glTextureStorage2D(tex->id_, (GLsizei)levels, (GLenum)internal_format, (GLsizei)width,
                       (GLsizei)height);
    tex->setWrapMode(TextureWrapMode::ClampToEdge);
    tex->setFilterMode(TextureFilterMode::Linear);
    tex->done();
    return tex;
}

void TextureCubemap::release()
{
    if (id_ > 0)
    {
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
}

void TextureCubemap::setWrapMode(TextureWrapMode mode)
{
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GLint(mode));
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GLint(mode));
    glTextureParameteri(id_, GL_TEXTURE_WRAP_R, GLint(mode));
}

void TextureCubemap::setWrapModeS(TextureWrapMode mode)
{
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GLint(mode));
}

void TextureCubemap::setWrapModeT(TextureWrapMode mode)
{
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GLint(mode));
}

void TextureCubemap::setWrapModeR(TextureWrapMode mode)
{
    glTextureParameteri(id_, GL_TEXTURE_WRAP_R, GLint(mode));
}

void TextureCubemap::setData(Side i, const unsigned char *data, size_t width, size_t height,
                             size_t level, TextureFormat format)
{
    assert(i >= 0 && i < 6);
    if (width != width_ || height != height_ || level >= levels_)
    {
        throw std::runtime_error(ERROR_IMAGE_TEXTURE_MISMATCH);
    }
    glTextureSubImage3D(id_, (GLint)level, 0, 0, i, (GLsizei)width_, (GLsizei)height_, 1,
                        (GLenum)format, GL_UNSIGNED_BYTE, data);
    width_ = width;
    height_ = height;
}

void TextureCubemap::setData(Side i, const Image &im, size_t level)
{
    TextureFormat format = TextureFormat::RGB;
    if (im.channels() == 1)
    {
        format = TextureFormat::Red;
    }
    else if (im.channels() == 3)
    {
        format = TextureFormat::RGB;
    }
    else if (im.channels() == 4)
    {
        format = TextureFormat::RGBA;
    }
    else
    {
        throw std::runtime_error(ERROR_IMAGE_CHANNELS_MISMATCH);
    }

    if (im.width() != width_ || im.height() != height_ || level >= levels_)
    {
        throw std::runtime_error(ERROR_IMAGE_TEXTURE_MISMATCH);
    }

    setData(i, im.pixels().data(), im.width(), im.height(), level, format);
}

size_t TextureCubemap::width() const
{
    return width_;
}

size_t TextureCubemap::height() const
{
    return height_;
}

size_t TextureCubemap::levels() const
{
    return levels_;
}

TextureInternalFormat TextureCubemap::internalFormat() const
{
    return internal_format_;
}

void TextureCubemap::use(size_t unit)
{
    if (seamless_)
    {
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }
    else
    {
        glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }
    unit_ = unit;
    glBindTextureUnit((GLenum)unit, id_);
}

void TextureCubemap::done()
{
    if (in_use_)
    {
        glActiveTexture(GL_TEXTURE0 + (GLenum)unit_);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        in_use_ = false;
    }
}

void TextureCubemap::setFilterModeMin(TextureFilterMode mode)
{
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, (GLint)mode);
}

void TextureCubemap::setFilterModeMag(TextureFilterMode mode)
{
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, (GLint)mode);
}

void TextureCubemap::setFilterMode(TextureFilterMode mode)
{
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, (GLint)mode);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, (GLint)mode);
}

void TextureCubemap::generateMipMap()
{
    glGenerateTextureMipmap(id_);
}

bool TextureCubemap::valid() const
{
    return id_ > 0;
}

GLenum TextureCubemap::target() const
{
    return GL_TEXTURE_CUBE_MAP;
}

GLenum TextureCubemap::target(Side side) const
{
    return GL_TEXTURE_CUBE_MAP_POSITIVE_X + side;
}

GLuint TextureCubemap::id() const
{
    return id_;
}

} // namespace rcube
