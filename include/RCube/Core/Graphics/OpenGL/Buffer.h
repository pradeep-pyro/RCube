#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <cassert>
#include <memory>
#include <vector>

namespace rcube
{

class Buffer
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
    void setData(const float *buf, size_t size)
    {
        assert(size == size_);
        use();
        glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(float), buf);
        done();
    }
    void setData(const std::vector<float> &buf)
    {
        setData(&buf[0], buf.size());
    }
    void setData(const std::vector<glm::vec3> &buf)
    {
        setData(glm::value_ptr(buf[0]), buf.size() * 3);
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

} // namespace rcube