#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "glad/glad.h"
#include "Texture.h"
#include <vector>

class Framebuffer {
public:
    Framebuffer();
    Framebuffer(size_t width, size_t height, TextureInternalFormat cfmt);
    Framebuffer(size_t width, size_t height, TextureInternalFormat cfmt,
                TextureInternalFormat dfmt);
    Framebuffer(const Framebuffer &other) = delete;
    ~Framebuffer() {
        cout << "Framebuffer::Destructor" << endl;
    }
    void use();
    void done() const;
    void release();
    GLuint id() const;
    unsigned int addColorAttachment(TextureInternalFormat format);
    Texture2D * colorAttachment(size_t i=0);
    size_t numColorAttachments() const;
    bool hasDepthStencilAttachment() const;
    void resize(int width, int height);
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

    unsigned int id_;
    size_t width_, height_;
    std::vector<std::unique_ptr<Texture2D>> colors_;
    std::unique_ptr<Texture2D> color0_;
    std::unique_ptr<Texture2D> depth_stencil_;
    bool has_depth_stencil_;
};

#endif // FRAMEBUFFER_H
