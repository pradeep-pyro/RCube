#include "Framebuffer.h"
#include <stdexcept>
#include "checkglerror.h"
#include <iostream>
using namespace std;

const std::string ERROR_FRAMEBUFFER_UNINITIALIZED = "Cannot use Framebuffer without initializing";

Framebuffer::Framebuffer() {
    has_depth_stencil_ = false;
    colors_.reserve(8);
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

std::shared_ptr<Framebuffer> Framebuffer::create(size_t width, size_t height) {
    auto fbo = std::make_shared<Framebuffer>();
    glGenFramebuffers(1, &fbo->id_);
    fbo->width_ = width;
    fbo->height_ = height;
    fbo->has_depth_stencil_ = false;
    checkGLError();
    return fbo;
}

Framebuffer::~Framebuffer() {
    release();
}

bool Framebuffer::initialized() const {
    return id_ > 0;
}

void Framebuffer::release() {
    glDeleteFramebuffers(1, &id_);
    for (auto &tex : colors_) {
        tex->release();
    }
    if (hasDepthStencilAttachment()) {
        depth_stencil_->release();
    }
    id_ = 0;
}

void Framebuffer::use() {
    if (!initialized()) {
        throw std::runtime_error(ERROR_FRAMEBUFFER_UNINITIALIZED);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

void Framebuffer::done() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::isComplete() const {
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::addColorAttachment(TextureInternalFormat internal_format) {
    use();
    auto tex = Texture2D::create(width_, height_, 1, internal_format);
    colors_.push_back(tex);
    unsigned int index = static_cast<unsigned int>(colors_.size());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D,
                           colors_[colors_.size() - 1]->id(), 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + index);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
    done();
}

void Framebuffer::addDepthAttachment(TextureInternalFormat internal_format) {
    use();
    depth_stencil_ = Texture2D::create(width_, height_, 1, internal_format);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           depth_stencil_->id(), 0);
    has_depth_stencil_ = true;
    done();
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
    return colors_[i].get();
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
