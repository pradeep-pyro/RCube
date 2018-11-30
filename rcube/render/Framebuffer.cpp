#include "Framebuffer.h"
#include <stdexcept>
#include "checkglerror.h"
#include <iostream>
using namespace std;

Framebuffer::Framebuffer() {
    cout << "Framebuffer::Framebuffer (0th)" << endl;
    glGenFramebuffers(1, &id_);
    has_depth_stencil_ = false;
}
Framebuffer::Framebuffer(size_t width, size_t height, TextureInternalFormat cfmt)
    : id_(0), width_(width), height_(height) {
    cout << "Framebuffer::Framebuffer (1st)" << endl;
    glGenFramebuffers(1, &id_);
    use();
    colors_.reserve(8);
    addColorAttachment(cfmt);  // Color attachment 0
    done();
    has_depth_stencil_ = false;
}

Framebuffer::Framebuffer(size_t width, size_t height, TextureInternalFormat cfmt, TextureInternalFormat dfmt)
    : id_(0), width_(width), height_(height) {
    cout << "Framebuffer::Framebuffer (2nd)" << endl;
    glGenFramebuffers(1, &id_);
    use();
    color0_ = std::make_unique<Texture2D>(cfmt);
    color0_->resize(width_, height_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           color0_->id(), 0);
    // Depth+Stencil attachment
    depth_stencil_ = std::make_unique<Texture2D>(dfmt);
    depth_stencil_->resize(width_, height_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           depth_stencil_->id(), 0);
    done();
    has_depth_stencil_ = true;
}

GLuint Framebuffer::id() const {
    return id_;
}

size_t Framebuffer::width() const {
    return width_;
}

size_t Framebuffer::height() const {
    return height_;
}

void Framebuffer::resize(int width, int height) {
    for (auto &col : colors_) {
        col->resize(width, height);
    }
    depth_stencil_->resize(width, height);
    width_ = width;
    height_ = height;
}

void Framebuffer::use() {
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

void Framebuffer::done() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::release() {
    if (id_ > 0) {
        glDeleteFramebuffers(1, &id_);
        id_ = 0;
    }
}

bool Framebuffer::isComplete() const {
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

unsigned int Framebuffer::addColorAttachment(TextureInternalFormat internal_format) {
    use();
    colors_.push_back(std::make_unique<Texture2D>(internal_format));
    unsigned int index = static_cast<unsigned int>(colors_.size());
    colors_[colors_.size() - 1]->use();
    colors_[colors_.size() - 1]->resize(width_, height_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D,
                           colors_[colors_.size() - 1]->id(), 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + index);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
    done();
    return index;
}

size_t Framebuffer::numColorAttachments() const {
    return colors_.size();
}

bool Framebuffer::hasDepthStencilAttachment() const {
    return has_depth_stencil_;
}

Texture2D * Framebuffer::colorAttachment(size_t i) {
    if (i >= colors_.size() + 1) {
        throw std::runtime_error("Invalid (out-of-range) index " + std::to_string(i) +
                                 "for color attachments; expected <= " + std::to_string(colors_.size()));
    }
    return i == 0 ? color0_.get() : colors_[i - 1].get();
}

void Framebuffer::blit(Framebuffer &target_fbo) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target_fbo.id());
    glDrawBuffer(GL_BACK);
    glBlitFramebuffer(0, 0, width_, height_, 0, 0, target_fbo.width(), target_fbo.height(),
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::blitToScreen(glm::ivec2 dst0, glm::ivec2 dst1, bool color,
                               bool depth, bool stencil) {
    GLbitfield bits = 0;
    if (color) {
        bits |= GL_COLOR_BUFFER_BIT;
    }
    if (depth) {
        bits |= GL_DEPTH_BUFFER_BIT;
    }
    if (stencil) {
        bits |= GL_STENCIL_BUFFER_BIT;
    }
    glBlitNamedFramebuffer(id_, 0, 0, 0, width_, height_,
                           dst0.x, dst0.y, dst1.x, dst1.y,
                           bits, GL_NEAREST);
    checkGLError();
}

void Framebuffer::blitToScreen(glm::ivec2 src0, glm::ivec2 src1, glm::ivec2 dst0,
                               glm::ivec2 dst1,  bool color, bool depth,
                               bool stencil) {
    GLbitfield bits = 0;
    if (color) {
        bits |= GL_COLOR_BUFFER_BIT;
    }
    if (depth) {
        bits |= GL_DEPTH_BUFFER_BIT;
    }
    if (stencil) {
        bits |= GL_STENCIL_BUFFER_BIT;
    }
    glBlitNamedFramebuffer(id_, 0, src0.x, src0.y, src1.x, src1.y,
                           dst0.x, dst0.y, dst1.x, dst1.y,
                           bits, GL_NEAREST);
}

Image Framebuffer::getImage(int attachment_index) const {
    Image im;
    unsigned char *pixel_data = (unsigned char*)malloc(3*width_*height_);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0+attachment_index);
    glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    im.setPixels(width_, height_, 3, pixel_data);
    free(pixel_data);
    return im;
}
