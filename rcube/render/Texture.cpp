#include "Texture.h"
#include <vector>
#include "glm/gtc/type_ptr.hpp"

using namespace std;


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

Texture2D::Texture2D(TextureInternalFormat internal_format) : Texture(internal_format), width_(0), height_(0) {
    use();
    setWrapMode(TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge);
    setFilterMode(TextureFilterMode::Linear, TextureFilterMode::Linear);
    done();
}

TextureTarget Texture2D::target() const {
    return TextureTarget::TwoD;
}

void Texture2D::setWrapMode(TextureWrapMode s, TextureWrapMode t) {
    use();
    glTexParameteri((GLint)target(), GL_TEXTURE_WRAP_S, (GLint)s);
    glTexParameteri((GLint)target(), GL_TEXTURE_WRAP_T, (GLint)t);
    done();
}

void Texture2D::setData(const unsigned char* data, size_t width, size_t height, TextureFormat format) {
    use();
    glTexImage2D((GLint)target(), 0, (GLint)internal_format_, width, height, 0, (GLint)format, GL_UNSIGNED_BYTE, data);
    width_ = width;
    height_ = height;
    generateMipMap();
    done();
}

void Texture2D::setData(const float* data, size_t width, size_t height, TextureFormat format) {
    use();
    glTexImage2D((GLint)target(), 0, (GLint)internal_format_, width, height, 0, (GLint)format, GL_FLOAT, data);
    width_ = width;
    height_ = height;
    generateMipMap();
    done();
}

void Texture2D::setData(const Image &im) {
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
        throw std::runtime_error("Unexpected number of channels, expected 1, 3, or 4");
    }
    setData(im.pixels().data(), im.width(), im.height(), format);
}

void Texture2D::resize(size_t width, size_t height) {
    use();
    glTexImage2D((GLint)target(), 0, (GLint)internal_format_, width, height, 0,
                 (GLint)getMatchingFormatForInternalFormat(internal_format_),
                 (GLint)getMatchingTypeForInternalFormat(internal_format_), nullptr);
    width_ = width;
    height_ = height;
    done();
}

size_t Texture2D::width() const {
    return width_;
}

size_t Texture2D::height() const {
    return height_;
}

// --------------------------------------
// TextureRectangle
// --------------------------------------
TextureRectangle::TextureRectangle(TextureInternalFormat internal_format) : Texture2D(internal_format) {
}

TextureTarget TextureRectangle::target() const {
    return TextureTarget::Rectangle;
}
void TextureRectangle::generateMipMap() {
    // Do nothing; Rectangle textures do not support mipmapping
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
    cout << "TextureCube Constructor: " << id_ << endl;
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
