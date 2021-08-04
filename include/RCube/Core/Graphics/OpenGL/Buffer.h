#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <cassert>
#include <memory>
#include <vector>

namespace rcube
{

enum class BufferType
{
    Array = GL_ARRAY_BUFFER,
    ElementArray = GL_ELEMENT_ARRAY_BUFFER,
    Uniform = GL_UNIFORM_BUFFER,
    PixelPack = GL_PIXEL_PACK_BUFFER,
};

template <BufferType Type> class Buffer
{
    GLuint id_ = 0;
    size_t size_ = 0;

  public:
    static std::shared_ptr<Buffer> create(size_t num_elements, GLenum usage = GL_DYNAMIC_DRAW)
    {
        auto buf = std::make_shared<Buffer>();
        glCreateBuffers(1, &buf->id_);
        buf->reserve(num_elements, usage);
        return buf;
    }
    ~Buffer()
    {
        if (id_ > 0)
        {
            release();
        }
    }
    BufferType type()
    {
        return Type;
    }
    void reserve(size_t bytes, GLenum usage = GL_DYNAMIC_DRAW)
    {
        glNamedBufferData(id_, bytes, NULL, usage);
        size_ = bytes;
    }

    void release()
    {
        glDeleteBuffers(1, &id_);
        id_ = 0;
    }
    template <BufferType T = Type,
              typename = std::enable_if<T == BufferType::Array || T == BufferType::Uniform>::type>
    void setData(const float *buf, size_t size, size_t offset = 0)
    {
        assert(size * sizeof(float) == size_);
        glNamedBufferSubData(id_, offset, size * sizeof(float), buf);
    }
    template <BufferType T = Type,
              typename = std::enable_if<T == BufferType::Array || T == BufferType::Uniform>::type>
    void setData(const int *buf, size_t size, size_t offset = 0)
    {
        assert(size * sizeof(int) == size_);
        glNamedBufferSubData(id_, offset, size * sizeof(int), buf);
    }
    template <BufferType T = Type,
              typename = std::enable_if<T == BufferType::Array || T == BufferType::Uniform>::type>
    void setData(const std::vector<float> &buf)
    {
        setData(&buf[0], buf.size());
    }
    template <BufferType T = Type, typename = std::enable_if<T == BufferType::Array>::type>
    void setData(const std::vector<glm::vec3> &buf)
    {
        setData(glm::value_ptr(buf[0]), buf.size() * 3);
    }
    template <BufferType T = Type, typename = std::enable_if<T == BufferType::Array>::type>
    void setData(const std::vector<glm::vec2> &buf)
    {
        setData(glm::value_ptr(buf[0]), buf.size() * 2);
    }
    template <BufferType T = Type, typename = std::enable_if<T == BufferType::ElementArray>::type>
    void setData(const unsigned int *buf, size_t size)
    {
        assert(size * sizeof(unsigned int) == size_);
        glNamedBufferSubData(id_, 0, size * sizeof(unsigned int), buf);
    }
    template <BufferType T = Type, typename = std::enable_if<T == BufferType::ElementArray>::type>
    void setData(const std::vector<unsigned int> &buf)
    {
        setData(&buf[0], buf.size());
    }
    template <BufferType T = Type, typename = std::enable_if<T == BufferType::ElementArray>::type>
    void setData(const std::vector<glm::uvec3> &buf)
    {
        setData(glm::value_ptr(buf[0]), buf.size() * 3);
    }
    template <BufferType T = Type, typename = std::enable_if<T == BufferType::ElementArray>::type>
    void setData(const std::vector<glm::uvec2> &buf)
    {
        setData(glm::value_ptr(buf[0]), buf.size() * 2);
    }
    template <BufferType T = Type, typename = std::enable_if<T == BufferType::PixelPack>::type>
    void *map()
    {
        void *mapped_buffer = glMapNamedBuffer(id_, GL_READ_ONLY);
        return mapped_buffer;
    }
    template <BufferType T = Type, typename = std::enable_if<T == BufferType::PixelPack>::type>
    void unmap()
    {
        glUnmapNamedBuffer(id_);
    }
    size_t size() const
    {
        return size_;
    }
    GLuint id() const
    {
        return id_;
    }
    void use() const
    {
        glBindBuffer(GLenum(Type), id_);
    }
    void done() const
    {
        glBindBuffer(GLenum(Type), 0);
    }
    template <BufferType T = Type, typename = std::enable_if<T == BufferType::Uniform>::type>
    void bindBase(int index) const
    {
        glBindBufferBase(GLenum(Type), index, id_);
    }
};

using ArrayBuffer = Buffer<BufferType::Array>;
using ElementArrayBuffer = Buffer<BufferType::ElementArray>;
using UniformBuffer = Buffer<BufferType::Uniform>;
using PixelPackBuffer = Buffer<BufferType::PixelPack>;

} // namespace rcube