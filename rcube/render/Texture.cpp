#include "Texture.h"
#include <vector>
#include "glm/gtc/type_ptr.hpp"

namespace rcube {

const std::string ERROR_IMAGE_TEXTURE_MISMATCH = "Image dimensions are different from allocated texture dimensions";
const std::string ERROR_IMAGE_CHANNELS_MISMATCH = "Unexpected number of channels, expected 1, 3, or 4";
const std::string ERROR_TEXTURE_UNINITIALIZED = "Cannot use texture without initializing";

// --------------------------------------
// Texture2D
// --------------------------------------

std::shared_ptr<Texture2D> Texture2D::create(size_t width, size_t height, size_t levels,
                                             TextureInternalFormat internal_format) {
    auto tex = std::make_shared<Texture2D>();
    tex->width_ = width;
    tex->height_ = height;
    tex->levels_ = levels;
    tex->internal_format_ = internal_format;
    tex->num_samples_ = 0;
    tex->target_ = GL_TEXTURE_2D;
    glGenTextures(1, &tex->id_);
    tex->use();
    glTexStorage2D(tex->target_, levels, (GLenum)internal_format, width, height);
    tex->setWrapMode(TextureWrapMode::ClampToEdge);
    tex->setFilterMode(TextureFilterMode::Linear);
    tex->done();
    return tex;
}

std::shared_ptr<Texture2D> Texture2D::createMS(size_t width, size_t height, size_t num_samples,
                                               TextureInternalFormat internal_format) {
    auto tex = std::make_shared<Texture2D>();
    tex->width_ = width;
    tex->height_ = height;
    tex->levels_ = 1;
    tex->internal_format_ = internal_format;
    tex->num_samples_ = num_samples;
    tex->target_ = GL_TEXTURE_2D_MULTISAMPLE;
    glGenTextures(1, &tex->id_);
    tex->use();
    glTexStorage2DMultisample(tex->target_, tex->num_samples_, (GLenum)internal_format, width, height, GL_TRUE);
        tex->setWrapMode(TextureWrapMode::ClampToEdge);
    tex->setFilterMode(TextureFilterMode::Linear);
    tex->done();
    return tex;
}

Texture2D::~Texture2D() {
    release();
}

bool Texture2D::valid() const {
    return id_ > 0;
}

void Texture2D::release() {
    if (valid()) {
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
}

void Texture2D::setBorderColor(const glm::vec4 &color) {
    if (num_samples_ > 0) {
        return;
    }
    use();
    glTexParameterfv(target_, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    done();
}

void Texture2D::setFilterMode(TextureFilterMode mode) {
    if (num_samples_ > 0) {
        return;
    }
    use();
    glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, (GLint)mode);
    glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, (GLint)mode);
    done();
}

void Texture2D::setFilterModeMin(TextureFilterMode mode) {
    if (num_samples_ > 0) {
        return;
    }
    use();
    glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, (GLint)mode);
    done();
}

void Texture2D::setFilterModeMag(TextureFilterMode mode) {
    if (num_samples_ > 0) {
        return;
    }
    use();
    glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, (GLint)mode);
    done();
}

void Texture2D::generateMipMap() {
    if (num_samples_ > 0) {
        return;
    }
    use();
    glGenerateMipmap(target_);
    done();
}

void Texture2D::setWrapMode(TextureWrapMode mode) {
    if (num_samples_ > 0) {
        return;
    }
    use();
    glTexParameteri(target_, GL_TEXTURE_WRAP_S, (GLint)mode);
    glTexParameteri(target_, GL_TEXTURE_WRAP_T, (GLint)mode);
    done();
}

void Texture2D::setWrapModeS(TextureWrapMode mode) {
    if (num_samples_ > 0) {
        return;
    }
    use();
    glTexParameteri(target_, GL_TEXTURE_WRAP_S, (GLint)mode);
    done();
}

void Texture2D::setWrapModeT(TextureWrapMode mode) {
    if (num_samples_ > 0) {
        return;
    }
    use();
    glTexParameteri(target_, GL_TEXTURE_WRAP_T, (GLint)mode);
    done();
}

void Texture2D::setData(const unsigned char* data, TextureFormat format, size_t level) {
    use();
    glTexSubImage2D(target_, level, 0, 0, width_, height_, (GLenum)format, GL_UNSIGNED_BYTE, data);
    generateMipMap();
    done();
}

void Texture2D::setData(const float* data, TextureFormat format, size_t level) {
    use();
    glTexSubImage2D(target_, level, 0, 0, width_, height_, (GLenum)format, GL_FLOAT, data);
    generateMipMap();
    done();
}

void Texture2D::setData(const Image &im) {
    if (im.width() != width_ || im.height() != height_) {
        throw std::invalid_argument(ERROR_IMAGE_TEXTURE_MISMATCH);
    }
    TextureFormat format;
    if (im.channels() == 3) {
        format = TextureFormat::RGB;
    }
    else if (im.channels() == 4) {
        format = TextureFormat::RGBA;
    }
    else if (im.channels() == 1) {
        format = TextureFormat::Red;
    }
    else {
        throw std::runtime_error(ERROR_IMAGE_CHANNELS_MISMATCH);
    }
    setData(im.pixels().data(), format);
}

void Texture2D::setData(const Image &im, TextureFormat fmt) {
    if (im.width() != width_ || im.height() != height_) {
        throw std::invalid_argument(ERROR_IMAGE_TEXTURE_MISMATCH);
    }
    setData(im.pixels().data(), fmt);
}

size_t Texture2D::width() const {
    return width_;
}

size_t Texture2D::height() const {
    return height_;
}

size_t Texture2D::levels() const {
    return levels_;
}

size_t Texture2D::numSamples() const {
    return num_samples_;
}

GLenum Texture2D::target() const {
    return target_;
}

TextureInternalFormat Texture2D::internalFormat() const {
    return internal_format_;
}

GLuint Texture2D::id() const {
    return id_;
}

void Texture2D::use(size_t unit) {
    if (!valid()) {
        throw std::runtime_error(ERROR_TEXTURE_UNINITIALIZED);
    }
    unit_ = unit;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(target_, id_);
}

void Texture2D::done() {
    if (in_use_) {
        glActiveTexture(GL_TEXTURE0 + unit_);
        glBindTexture(target_, 0);
        in_use_ = false;
    }
}

// --------------------------------------
// TextureCube
// --------------------------------------
TextureCubemap::~TextureCubemap() {
    release();
}

std::shared_ptr<TextureCubemap> TextureCubemap::create(size_t width, size_t height, size_t levels, bool seamless,
                                                       TextureInternalFormat internal_format) {
    auto tex = std::make_shared<TextureCubemap>();
    tex->width_ = width;
    tex->height_ = height;
    tex->levels_ = levels;
    tex->internal_format_ = internal_format;
    tex->seamless_ = seamless;
    glGenTextures(1, &tex->id_);
    tex->use();
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, levels, (GLenum)internal_format, width, height);
    tex->setWrapMode(TextureWrapMode::ClampToEdge);
    tex->setFilterMode(TextureFilterMode::Linear);
    tex->done();
    return tex;
}

void TextureCubemap::release() {
    if (id_ > 0) {
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
}

void TextureCubemap::setWrapMode(TextureWrapMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GLint(mode));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GLint(mode));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GLint(mode));
    done();
}

void TextureCubemap::setWrapModeS(TextureWrapMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GLint(mode));
    done();
}

void TextureCubemap::setWrapModeT(TextureWrapMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GLint(mode));
    done();
}

void TextureCubemap::setWrapModeR(TextureWrapMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GLint(mode));
    done();
}

void TextureCubemap::setData(int i, const unsigned char *data, size_t width, size_t height, size_t level,
                             TextureFormat format) {
    assert(i >= 0 && i < 6);
    if (width != width_ || height != height_ || level >= levels_) {
        throw std::runtime_error(ERROR_IMAGE_TEXTURE_MISMATCH);
    }
    use();
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, level, 0, 0, width_, height_, (GLenum)format, GL_UNSIGNED_BYTE, data);
    width_ = width;
    height_ = height;
    done();
}

void TextureCubemap::setData(int i, const Image &im, size_t level) {
    TextureFormat format = TextureFormat::RGB;
    if (im.channels() == 1) {
        format =  TextureFormat::Red;
    }
    else if (im.channels() == 3) {
        format =  TextureFormat::RGB;
    }
    else if (im.channels() == 4) {
        format =  TextureFormat::RGBA;
    }
    else {
        throw std::runtime_error(ERROR_IMAGE_CHANNELS_MISMATCH);
    }

    if (im.width() != width_ || im.height() != height_ || level >= levels_) {
        throw std::runtime_error(ERROR_IMAGE_TEXTURE_MISMATCH);
    }

    setData(i, im.pixels().data(), im.width(), im.height(), level, format);
}

void TextureCubemap::use(size_t unit) {
    if (seamless_) {
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }
    else {
        glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }
    unit_ = unit;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
}

void TextureCubemap::done() {
    if (in_use_) {
        glActiveTexture(GL_TEXTURE0 + unit_);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        in_use_ = false;
    }
}

void TextureCubemap::setFilterModeMin(TextureFilterMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)mode);
    done();
}

void TextureCubemap::setFilterModeMag(TextureFilterMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)mode);
    done();
}

void TextureCubemap::setFilterMode(TextureFilterMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)mode);
    done();
}

void TextureCubemap::generateMipMap() {
    use();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    done();
}

bool TextureCubemap::valid() const {
    return id_ > 0;
}

GLenum TextureCubemap::target() const {
    return GL_TEXTURE_CUBE_MAP;
}

GLenum TextureCubemap::target(Side side) const {
    return GL_TEXTURE_CUBE_MAP_POSITIVE_X + side;
}

GLuint TextureCubemap::id() const {
    return id_;
}

} // namespace rcube
