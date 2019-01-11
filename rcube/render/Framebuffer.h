#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "glad/glad.h"
#include "Texture.h"
#include <vector>

namespace rcube {

/**
 * Framebuffer represents a collection of color, depth and stencil buffers that can be
 * used for off-screen rendering.
 * Use the Framebuffer::create(...) static method to create a new framebuffer.
 */
class Framebuffer {
public:
    Framebuffer();
    Framebuffer(const Framebuffer &other) = delete;
    ~Framebuffer();
    /**
     * Creates a new framebuffer
     * @param width Width of color and depth buffers
     * @param height Height of color and depth buffers
     * @return Shared pointer to Framebuffer
     */
    static std::shared_ptr<Framebuffer> create(size_t width, size_t height);
    /**
     * Returns whether the Framebuffer is actually created and initialized on the GPU
     * @return Whether initialized on GPU
     */
    bool initialized() const;
    /**
     * Releases the resources associated to the framebuffer from the GPU
     */
    void release();
    /**
     * Bind and use the framebuffer
     */
    void use();
    /**
     * Unbind the framebuffer
     */
    void done() const;
    /**
     * OpenGL ID of the framebuffer
     * @return
     */
    GLuint id() const;

    /**
     * Add another color attachment
     * @param format Format of the color buffer in the GPU
     */
    void addColorAttachment(TextureInternalFormat format);

    /**
     * Add/replace the depth attachment
     * @param format Format of the depth buffer in the GPU
     */
    void addDepthAttachment(TextureInternalFormat format);

    /**
     * Returns the ith color attachment
     * @param i Index of the color attachment
     * @return Pointer to Texture2D
     */
    Texture2D * colorAttachment(size_t i=0);

    /**
     * Returns the number of color attachments in this framebuffer
     * @return Number of color attachments
     */
    size_t numColorAttachments() const;

    /**
     * Returns whether there is depth/stencil attachment in this framebuffer
     * @return
     */
    bool hasDepthStencilAttachment() const;

    /**
     * Width of the framebuffer
     * @return Width
     */
    size_t width() const;

    /**
     * Height of the framebuffer
     * @return Height
     */
    size_t height() const;

    /**
     * Returns whether the framebuffer is completely defined
     * @return Whether complete
     */
    bool isComplete() const;

    /**
     * Returns an image representing the color attachment
     * @param attachment_index Index of the color attachment
     * @return Image
     */
    Image getImage(int attachment_index=0) const;
    /**
     * Blits (copies) data from the current framebuffer to the target
     * Only works if both framebuffers match in dimensions
     * @param target_fbo
     */
    void blit(Framebuffer &target_fbo);

    /**
     * Blits (copies) data from the current framebuffer to the default framebuffer
     * @param dst0 ROI denoted by bottom-right point in current framebuffer
     * @param dst1 ROI denoted by bottom-right point in destination framebuffer
     * @param color Whether to copy color buffers
     * @param depth Whether to copy depth buffer
     * @param stencil Whether to copy stencil buffer
     */

    void blitToScreen(glm::ivec2 dst0, glm::ivec2 dst1, bool color=true,
                      bool depth=true, bool stencil=true);
    /**
     * Blits (copies) data from the current framebuffer to the default framebuffer
     * @param src0 ROI denoted by top-left point in current framebuffer
     * @param src1 ROI denoted by top-left point in destination framebuffer
     * @param dst0 ROI denoted by bottom-right point in current framebuffer
     * @param dst1 ROI denoted by bottom-right point in destination framebuffer
     * @param color Whether to copy color buffers
     * @param depth Whether to copy depth buffer
     * @param stencil Whether to copy stencil buffer
     */

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

} // namespace rcube

#endif // FRAMEBUFFER_H
