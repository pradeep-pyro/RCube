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
    ElementArray = GL_ELEMENT_ARRAY_BUFFER
};

template <BufferType Type> class Buffer
{
    GLuint id_;
    size_t size_ = 0;

  public:
    static std::shared_ptr<Buffer> create(size_t num_elements)
    {
        auto buf = std::make_shared<Buffer>();
        glGenBuffers(1, &buf->id_);
        buf->reserve(num_elements);
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
    void reserve(size_t num_elements)
    {
        use();
        glBufferData(GL_ARRAY_BUFFER, num_elements * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        size_ = num_elements;
        done();
    }
    void release()
    {
        glDeleteBuffers(1, &id_);
        id_ = 0;
    }
    template <BufferType T = Type>
    std::enable_if<T == BufferType::Array, void> setData(const float *buf, size_t size)
    {
        assert(size == size_);
        use();
        glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(float), buf);
        done();
    }
    template <BufferType T = Type>
    std::enable_if<T == BufferType::Array, void> setData(const std::vector<float> &buf)
    {
        setData(&buf[0], buf.size());
    }
    template <BufferType T = Type>
    std::enable_if<T == BufferType::Array, void> setData(const std::vector<glm::vec3> &buf)
    {
        setData(glm::value_ptr(buf[0]), buf.size() * 3);
    }
    template <BufferType T = Type>
    std::enable_if<T == BufferType::Array, void> setData(const std::vector<glm::vec2> &buf)
    {
        setData(glm::value_ptr(buf[0]), buf.size() * 2);
    }
    template <BufferType T = Type>
    std::enable_if<T == BufferType::ElementArray, void> setData(const unsigned int *buf, size_t size)
    {
        assert(size == size_);
        use();
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size * sizeof(unsigned int), buf);
        done();
    }
    template <BufferType T = Type>
    std::enable_if<T == BufferType::ElementArray, void>
    setData(const std::vector<unsigned int> &buf)
    {
        setData(&buf[0], buf.size());
    }
    template <BufferType T = Type>
    std::enable_if<T == BufferType::ElementArray, void> setData(const std::vector<glm::uvec3> &buf)
    {
        setData(glm::value_ptr(buf[0]), buf.size() * 3);
    }
    template <BufferType T = Type>
    std::enable_if<T == BufferType::ElementArray, void> setData(const std::vector<glm::uvec2> &buf)
    {
        setData(glm::value_ptr(buf[0]), buf.size() * 2);
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
        glBindBuffer(GL_ARRAY_BUFFER, id_);
    }
    void done() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};

using ArrayBuffer = Buffer<BufferType::Array>;
using ElementArrayBuffer = Buffer<BufferType::ElementArray>;

} // namespace rcube