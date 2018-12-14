#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "glad/glad.h"
#include "Texture.h"
#include <vector>

class Framebuffer {
public:
    Framebuffer();
    Framebuffer(const Framebuffer &other) = delete;
    ~Framebuffer();
    static std::shared_ptr<Framebuffer> create(size_t width, size_t height);
    bool initialized() const;
    void release();
    void use();
    void done() const;
    GLuint id() const;
    void addColorAttachment(TextureInternalFormat format);
    void addDepthAttachment(TextureInternalFormat format);
    Texture2D * colorAttachment(size_t i=0);
    size_t numColorAttachments() const;
    bool hasDepthStencilAttachment() const;
    size_t width() const;
    size_t height() const;
    bool isComplete() const;
    Image getImage(int attachment_index=0) const;
    void blit(Framebuffer &target_fbo);
    void blitToScreen(glm::ivec2 dst0, glm::ivec2 dst1, bool color=true,
                      bool depth=true, bool stencil=true);
    void blitToScreen(glm::ivec2 src0, glm::ivec2 src1, glm::ivec2 dst0,
                      glm::ivec2 dst1, bool color=true, bool depth=true,
                      bool stencil=true);
private:
    void addAttachment(GLenum attachment, GLenum data_type, unsigned int *attachment_id);

    unsigned int id_ = 0;
    size_t width_ = 0;
    size_t height_ = 0;
    std::vector<std::shared_ptr<Texture2D>> colors_;
    std::shared_ptr<Texture2D> depth_stencil_;
    bool has_depth_stencil_ = false;
};

#endif // FRAMEBUFFER_H
