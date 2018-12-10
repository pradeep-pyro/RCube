#include "Texture.h"
#include <vector>
#include "glm/gtc/type_ptr.hpp"
#include "checkglerror.h"

const std::string ERROR_IMAGE_TEXTURE_MISMATCH = "Image dimensions are different from allocated texture dimensions";
const std::string ERROR_IMAGE_CHANNELS_MISMATCH = "Unexpected number of channels, expected 1, 3, or 4";
const std::string ERROR_TEXTURE_UNINITIALIZED = "Cannot use texture without initializing";

TextureFormat getMatchingFormatForInternalFormat(TextureInternalFormat ifmt) {
    switch (ifmt) {
    case TextureInternalFormat::Depth16:
    case TextureInternalFormat::Depth24:
    case TextureInternalFormat::Depth32:
    case TextureInternalFormat::Depth32F:
        return TextureFormat::Depth;

    case TextureInternalFormat::Depth24Stencil8:
    case TextureInternalFormat::Depth32FStencil8:
        return TextureFormat::DepthStencil;

    case TextureInternalFormat::R8:
    case TextureInternalFormat::R16:
    case TextureInternalFormat::R16F:
    case TextureInternalFormat::R32F:
        return TextureFormat::Red;

    case TextureInternalFormat::RG8:
    case TextureInternalFormat::RG16:
    case TextureInternalFormat::RG16F:
    case TextureInternalFormat::RG32F:
        return TextureFormat::RG;

    case TextureInternalFormat::RGB8:
    case TextureInternalFormat::RGB16:
    case TextureInternalFormat::RGB16F:
    case TextureInternalFormat::RGB32F:
        return TextureFormat::BGR;

    case TextureInternalFormat::RGBA8:
    case TextureInternalFormat::RGBA16:
    case TextureInternalFormat::RGBA16F:
    case TextureInternalFormat::RGBA32F:
        return TextureFormat::BGRA;
    }
}

GLenum getMatchingTypeForInternalFormat(TextureInternalFormat ifmt) {
    switch (ifmt) {
    case TextureInternalFormat::Depth16:
    case TextureInternalFormat::Depth24:
    case TextureInternalFormat::Depth32:
        return GL_UNSIGNED_BYTE;
    case TextureInternalFormat::Depth32F:
        return GL_FLOAT;

    case TextureInternalFormat::Depth24Stencil8:
        return GL_UNSIGNED_INT_24_8;
    case TextureInternalFormat::Depth32FStencil8:
        return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;

    case TextureInternalFormat::R8:
    case TextureInternalFormat::R16:
        return GL_UNSIGNED_BYTE;
    case TextureInternalFormat::R16F:
    case TextureInternalFormat::R32F:
        return GL_FLOAT;

    case TextureInternalFormat::RG8:
    case TextureInternalFormat::RG16:
        return GL_UNSIGNED_BYTE;
    case TextureInternalFormat::RG16F:
    case TextureInternalFormat::RG32F:
        return GL_FLOAT;

    case TextureInternalFormat::RGB8:
    case TextureInternalFormat::RGB16:
        return GL_UNSIGNED_BYTE;
    case TextureInternalFormat::RGB16F:
    case TextureInternalFormat::RGB32F:
        return GL_FLOAT;

    case TextureInternalFormat::RGBA8:
    case TextureInternalFormat::RGBA16:
        return GL_UNSIGNED_BYTE;
    case TextureInternalFormat::RGBA16F:
    case TextureInternalFormat::RGBA32F:
        return GL_FLOAT;
    }
}


// --------------------------------------
// Texture (base class)
// --------------------------------------

Texture::Texture(TextureInternalFormat internal_format)
    : internal_format_(internal_format), in_use_(false) {
    glGenTextures(1, &id_);
}

Texture::~Texture() {
    //free();
}

void Texture::use(unsigned int unit) {
    unit_ = unit;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture((GLint)target(), id_);
    in_use_ = true;
}

void Texture::done() {
    if (in_use_) {
        glActiveTexture(GL_TEXTURE0 + unit_);
        glBindTexture((GLint)target(), 0);
        in_use_ = false;
    }
}

GLuint Texture::id() const {
    return id_;
}

void Texture::setBorderColor(const glm::vec4 &color) {
    use();
    glTexParameterfv((GLint)target(), GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    done();
}

void Texture::setFilterMode(TextureFilterMode min, TextureFilterMode mag) {
    use();
    glTexParameteri((GLint)target(), GL_TEXTURE_MIN_FILTER, (GLint)min);
    glTexParameteri((GLint)target(), GL_TEXTURE_MIN_FILTER, (GLint)mag);
    done();
}

void Texture::generateMipMap() {
    use();
    glGenerateMipmap((GLint)target());
    done();
}

void Texture::free() {
    if (id_ > 0) {
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
}

// --------------------------------------
// Texture1D
// --------------------------------------
Texture1D::Texture1D(TextureInternalFormat internal_format) : Texture(internal_format), width_(0) {
    use();
    setWrapMode(TextureWrapMode::ClampToEdge);
    setFilterMode(TextureFilterMode::Linear, TextureFilterMode::Linear);
    done();
}

TextureTarget Texture1D::target() const {
    return TextureTarget::OneD;
}

void Texture1D::setWrapMode(TextureWrapMode mode) {
    use();
    glTexParameteri((GLint)target(), GL_TEXTURE_WRAP_S, (GLint)mode);
    done();
}

void Texture1D::setData(const float *data, size_t width, TextureFormat format) {
    use();
    glTexImage1D((GLint)target(), 0, (GLint)internal_format_, width, 0, (GLint)format, GL_FLOAT, data);
    width_ = width;
    done();
}

size_t Texture1D::width() const {
    return width_;
}

// --------------------------------------
// Texture2D
// --------------------------------------

Texture2D::Texture2D() : id_(0) {
}

void Texture2D::initialize(size_t width, size_t height, size_t levels, TextureInternalFormat internal_format) {
    if (valid()) {
        return;
    }
    width_ = width;
    height_ = height;
    levels_ = levels;
    internal_format_ = internal_format;
    glGenTextures(1, &id_);
    use();
    glTexStorage2D(GL_TEXTURE_2D, levels_, (GLenum)internal_format_, width_, height_);
    setWrapMode(TextureWrapMode::ClampToEdge);
    setFilterMode(TextureFilterMode::Linear);
    done();
    checkGLError();
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
    use();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    done();
}

void Texture2D::setFilterMode(TextureFilterMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)mode);
    done();
}

void Texture2D::setFilterModeMin(TextureFilterMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)mode);
    done();
}

void Texture2D::setFilterModeMag(TextureFilterMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)mode);
    done();
}

void Texture2D::generateMipMap() {
    use();
    glGenerateMipmap(GL_TEXTURE_2D);
    done();
}

void Texture2D::setWrapMode(TextureWrapMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)mode);
    done();
}

void Texture2D::setWrapModeS(TextureWrapMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)mode);
    done();
}

void Texture2D::setWrapModeT(TextureWrapMode mode) {
    use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)mode);
    done();
}

void Texture2D::setData(const unsigned char* data, TextureFormat format, size_t level) {
    use();
    glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, width_, height_, (GLenum)format, GL_UNSIGNED_BYTE, data);
    generateMipMap();
    done();
}

void Texture2D::setData(const float* data, TextureFormat format, size_t level) {
    use();
    glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, width_, height_, (GLenum)format, GL_FLOAT, data);
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

size_t Texture2D::width() const {
    return width_;
}

size_t Texture2D::height() const {
    return height_;
}

size_t Texture2D::levels() const {
    return levels_;
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
    glBindTexture(GL_TEXTURE_2D, id_);
}

void Texture2D::done() {
    if (in_use_) {
        glActiveTexture(GL_TEXTURE0 + unit_);
        glBindTexture(GL_TEXTURE_2D, 0);
        in_use_ = false;
    }
}

// --------------------------------------
// Texture3D
// --------------------------------------
Texture3D::Texture3D(TextureInternalFormat internal_format) : Texture(internal_format), width_(0), height_(0), depth_(0) {
    use();
    setWrapMode(TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge);
    setFilterMode(TextureFilterMode::Linear, TextureFilterMode::Linear);
    done();
}

TextureTarget Texture3D::target() const {
    return TextureTarget::ThreeD;
}

void Texture3D::setWrapMode(TextureWrapMode s, TextureWrapMode t, TextureWrapMode r) {
    use();
    glTexParameteri((GLint)target(), GL_TEXTURE_WRAP_S, (GLint)s);
    glTexParameteri((GLint)target(), GL_TEXTURE_WRAP_T, (GLint)t);
    glTexParameteri((GLint)target(), GL_TEXTURE_WRAP_R, (GLint)r);
    done();
}

void Texture3D::setData(const unsigned char *data, size_t width, size_t height, size_t depth, TextureFormat format) {
    use();
    glTexImage3D((GLint)target(), 0, (GLint)internal_format_, width, height, depth, 0, (GLint)format, GL_UNSIGNED_BYTE, data);
    width_ = width;
    height_ = height;
    depth_ = depth;
    done();
}

size_t Texture3D::width() const {
    return width_;
}

size_t Texture3D::height() const {
    return height_;
}

size_t Texture3D::depth() const {
    return depth_;
}

// --------------------------------------
// TextureCube
// --------------------------------------
TextureCube::TextureCube(TextureInternalFormat internal_format) : Texture(internal_format), width_(0), height_(0) {
    use();
    setFilterMode(TextureFilterMode::Linear, TextureFilterMode::Linear);
    glTexParameteri(GLint(target()), GL_TEXTURE_WRAP_S, GLint(TextureWrapMode::ClampToEdge));
    glTexParameteri(GLint(target()), GL_TEXTURE_WRAP_T, GLint(TextureWrapMode::ClampToEdge));
    glTexParameteri(GLint(target()), GL_TEXTURE_WRAP_R, GLint(TextureWrapMode::ClampToEdge));
    done();
}

void TextureCube::setData(int i, const unsigned char *data, size_t width, size_t height, TextureFormat format) {
    assert(i >= 0 && i < 6);
    use();
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, (GLint)internal_format_, width, height,
                 0, GLint(format), GL_UNSIGNED_BYTE, data);
    width_ = width;
    height_ = height;
    done();
}

void TextureCube::setData(int i, const Image &im) {
    TextureFormat format = TextureFormat::RGB;
    if (im.channels() == 1) {
        format =  TextureFormat::Red;
    }
    else if (im.channels() == 4) {
        format =  TextureFormat::RGBA;
    }
    setData(i, im.pixels().data(), im.width(), im.height(), format);
}

TextureTarget TextureCube::target() const {
    return TextureTarget::CubeMap;
}
