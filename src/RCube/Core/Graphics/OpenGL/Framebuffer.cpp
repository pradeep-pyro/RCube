#include "RCube/Core/Graphics/OpenGL/Framebuffer.h"
#include <iostream>
#include <stdexcept>

namespace rcube
{

const std::string ERROR_FRAMEBUFFER_UNINITIALIZED = "Cannot use Framebuffer without initializing";
const std::string ERROR_FRAMEBUFFER_NOT_COMPLETE = "Cannot use Framebuffer that is incomplete";

Framebuffer::Framebuffer()
{
    colors_.reserve(8);
    depth_stencil_ = nullptr;
}

GLuint Framebuffer::id() const
{
    return id_;
}

void Framebuffer::setDrawBuffers(const std::vector<int> &attachment_indices)
{
    std::vector<GLenum> draw_bufs;
    draw_bufs.reserve(attachment_indices.size());
    for (int att : attachment_indices)
    {
        draw_bufs.push_back(GL_COLOR_ATTACHMENT0 + att);
    }
    glNamedFramebufferDrawBuffers(id_, (GLsizei)draw_bufs.size(), draw_bufs.data());
}

void Framebuffer::setReadBuffer(int attachment_index)
{
    glNamedFramebufferReadBuffer(id_, GL_COLOR_ATTACHMENT0 + attachment_index);
}

std::shared_ptr<Framebuffer> Framebuffer::create()
{
    auto fbo = std::make_shared<Framebuffer>();
    glCreateFramebuffers(1, &fbo->id_);
    fbo->colors_.resize(8, nullptr);
    return fbo;
}

Framebuffer::~Framebuffer()
{
    release();
}

bool Framebuffer::initialized() const
{
    return id_ > 0;
}

void Framebuffer::release()
{
    glDeleteFramebuffers(1, &id_);
    id_ = 0;
}

void Framebuffer::use()
{
    if (!initialized())
    {
        throw std::runtime_error(ERROR_FRAMEBUFFER_UNINITIALIZED);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

void Framebuffer::useForWrite()
{
    if (!initialized())
    {
        throw std::runtime_error(ERROR_FRAMEBUFFER_UNINITIALIZED);
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id_);
}

void Framebuffer::useForRead()
{
    if (!initialized())
    {
        throw std::runtime_error(ERROR_FRAMEBUFFER_UNINITIALIZED);
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
}
void Framebuffer::done() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::isComplete()
{
    auto status = glCheckNamedFramebufferStatus(id_, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer not complete: " << status << std::endl;
    }
    return status == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::setColorAttachment(size_t index, std::shared_ptr<Texture2D> tex)
{
    assert(tex->valid());
    glNamedFramebufferTexture(id_, GL_COLOR_ATTACHMENT0 + (GLenum)index, tex->id(), 0);
    colors_[index] = tex;
}

void Framebuffer::resize(size_t width, size_t height)
{
    for (size_t i = 0; i < colors_.size(); ++i)
    {
        if (colors_[i] == nullptr)
        {
            continue;
        }
        auto new_tex =
            Texture2D::create(width, height, colors_[i]->levels(), colors_[i]->internalFormat());
        colors_[i]->release();
        setColorAttachment(i, new_tex);
    }
    if (depth_stencil_ != nullptr)
    {
        auto new_tex = Texture2D::create(width, height, 1 /*mipmap is always 1*/,
                                         depth_stencil_->internalFormat());
        depth_stencil_->release();
        setDepthAttachment(new_tex);
    }
}

void Framebuffer::setDepthAttachment(std::shared_ptr<Texture2D> tex)
{
    auto internal_format = tex->internalFormat();
    assert(internal_format == TextureInternalFormat::Depth16 ||
           internal_format == TextureInternalFormat::Depth24 ||
           internal_format == TextureInternalFormat::Depth32 ||
           internal_format == TextureInternalFormat::Depth32F);
    glNamedFramebufferTexture(id_, GL_DEPTH_ATTACHMENT, tex->id(), 0);
    depth_stencil_ = tex;
}

void Framebuffer::setDepthStencilAttachment(std::shared_ptr<Texture2D> tex)
{
    auto internal_format = tex->internalFormat();
    assert(internal_format == TextureInternalFormat::Depth24Stencil8 ||
           internal_format == TextureInternalFormat::Depth32FStencil8);
    glNamedFramebufferTexture(id_, GL_DEPTH_STENCIL_ATTACHMENT, tex->id(), 0);
    depth_stencil_ = tex;
}

size_t Framebuffer::numColorAttachments() const
{
    size_t count = 0;
    for (auto tex : colors_)
    {
        if (tex != nullptr)
        {
            ++count;
        }
    }
    return count;
}

bool Framebuffer::hasDepthStencilAttachment() const
{
    return depth_stencil_ != nullptr;
}

std::shared_ptr<Texture2D> Framebuffer::colorAttachment(size_t i)
{
    if (i >= colors_.size() + 1)
    {
        throw std::runtime_error(
            "Invalid (out-of-range) index " + std::to_string(i) +
            "for color attachments; expected <= " + std::to_string(colors_.size()));
    }
    return colors_[i];
}

void Framebuffer::blit(Framebuffer &target_fbo, glm::ivec2 src_origin, glm::ivec2 src_size,
                       glm::ivec2 dst_origin, glm::ivec2 dst_size, bool color, bool depth,
                       bool stencil)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target_fbo.id());
    GLbitfield bits = 0;
    if (color)
    {
        bits |= GL_COLOR_BUFFER_BIT;
    }
    if (depth && target_fbo.hasDepthStencilAttachment())
    {
        bits |= GL_DEPTH_BUFFER_BIT;
    }
    if (stencil && target_fbo.hasDepthStencilAttachment())
    {
        bits |= GL_STENCIL_BUFFER_BIT;
    }
    glBlitFramebuffer((GLint)src_origin.x, (GLint)src_origin.y, (GLint)src_size.x,
                      (GLint)src_size.y, (GLint)dst_origin.x, (GLint)dst_origin.y,
                      (GLint)dst_size.x, (GLint)dst_size.y, bits, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::blitToScreen(glm::ivec2 dst_origin, glm::ivec2 dst_size, bool color, bool depth,
                               bool stencil)
{
    GLbitfield bits = 0;
    if (color)
    {
        bits |= GL_COLOR_BUFFER_BIT;
    }
    if (depth)
    {
        bits |= GL_DEPTH_BUFFER_BIT;
    }
    if (stencil)
    {
        bits |= GL_STENCIL_BUFFER_BIT;
    }
    // glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glBlitFramebuffer((GLint)dst_origin.x, (GLint)dst_origin.y, (GLint)dst_size.x,
                      (GLint)dst_size.y, (GLint)dst_origin.x, (GLint)dst_origin.y,
                      (GLint)dst_size.x, (GLint)dst_size.y, bits, GL_NEAREST);
}

void Framebuffer::blitToScreen(glm::ivec2 src_origin, glm::ivec2 src_size, glm::ivec2 dst_origin,
                               glm::ivec2 dst_size, bool color, bool depth, bool stencil)
{
    GLbitfield bits = 0;
    if (color)
    {
        bits |= GL_COLOR_BUFFER_BIT;
    }
    if (depth)
    {
        bits |= GL_DEPTH_BUFFER_BIT;
    }
    if (stencil)
    {
        bits |= GL_STENCIL_BUFFER_BIT;
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glBlitFramebuffer(src_origin.x, src_origin.y, src_size.x, src_size.y, dst_origin.x,
                      dst_origin.y, dst_size.x, dst_size.y, bits, GL_NEAREST);
}

Image Framebuffer::getImage(int attachment_index) const
{
    assert(attachment_index < colors_.size());
    Image im;
    const size_t width = colors_[attachment_index]->width();
    const size_t height = colors_[attachment_index]->height();
    unsigned char *pixel_data = (unsigned char *)malloc(3 * width * height);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, id_);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment_index);
    glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    im.setPixels((int)width, (int)height, 3, pixel_data);
    free(pixel_data);
    return im;
}

} // namespace rcube
