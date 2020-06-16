#pragma once

#include "RCube/Core/Graphics/OpenGL/Buffer.h"
#include "glm/glm.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace rcube
{

enum AttributeLocation
{
    POSITION = 0,
    NORMAL,
    UV,
    COLOR,
    TANGENT
};

class AttributeBuffer
{
    std::string name_;
    GLuint location_ = 0;
    size_t dim_ = 1;
    std::vector<float> data_;
    std::shared_ptr<ArrayBuffer> buffer_;

  public:
    AttributeBuffer() = default;

    AttributeBuffer(const AttributeBuffer &other) = delete;

    AttributeBuffer &operator=(const AttributeBuffer &other) = delete;

    static std::shared_ptr<AttributeBuffer> create(std::string name, GLuint location, size_t dim)
    {
        auto attr_buf = std::make_shared<AttributeBuffer>();
        attr_buf->name_ = name;
        attr_buf->dim_ = dim;
        attr_buf->location_ = location;
        attr_buf->buffer_ = ArrayBuffer::create(1);
        return attr_buf;
    }

    const std::string &name() const
    {
        return name_;
    }

    const float *ptr() const
    {
        return &data_[0];
    }

    float *ptr()
    {
        return &data_[0];
    }

    const glm::vec3 *ptrVec3() const
    {
        if (dim_ != 3)
        {
            throw std::runtime_error("Attempting to interpret " + std::to_string(dim_) +
                                     "D data as 3D");
        }
        return reinterpret_cast<const glm::vec3 *>(&data_[0]);
    }

    glm::vec3 *ptrVec3()
    {
        if (dim_ != 3)
        {
            throw std::runtime_error("Attempting to interpret " + std::to_string(dim_) +
                                     "D data as 3D");
        }
        return reinterpret_cast<glm::vec3 *>(&data_[0]);
    }

    const glm::vec2 *ptrVec2() const
    {
        if (dim_ != 2)
        {
            throw std::runtime_error("Attempting to interpret " + std::to_string(dim_) +
                                     "D data as 2D");
        }
        return reinterpret_cast<const glm::vec2 *>(&data_[0]);
    }

    glm::vec2 *ptrVec2()
    {
        if (dim_ != 2)
        {
            throw std::runtime_error("Attempting to interpret " + std::to_string(dim_) +
                                     "D data as 2D");
        }
        return reinterpret_cast<glm::vec2 *>(&data_[0]);
    }

    size_t size() const
    {
        return data_.size();
    }

    size_t dim() const
    {
        return dim_;
    }

    std::shared_ptr<ArrayBuffer> buffer() const
    {
        return buffer_;
    }

    GLuint location() const
    {
        return location_;
    }

    const std::vector<float> &data() const
    {
        return data_;
    }

    std::vector<float> &data()
    {
        return data_;
    }

    void setData(const std::vector<float> &data)
    {
        if (data.size() % dim_ != 0)
        {
            throw std::runtime_error("Attempting to set data of size " +
                                     std::to_string(data.size()) + "which is not divisible by " +
                                     std::to_string(dim_) + " (dim)");
        }
        data_ = data;
    }

    void setData(const std::vector<glm::vec2> &data)
    {
        if (dim_ != 2)
        {
            throw std::runtime_error("Attempting to set 2D data, expected " + std::to_string(dim_) +
                                     "D data");
        }
        data_.assign(glm::value_ptr(data[0]), glm::value_ptr(data[0]) + data.size() * 2);
    }

    void setData(const std::vector<glm::vec3> &data)
    {
        if (dim_ != 3)
        {
            throw std::runtime_error("Attempting to set 3D data, expected " + std::to_string(dim_) +
                                     "D data");
        }
        data_.assign(glm::value_ptr(data[0]), glm::value_ptr(data[0]) + data.size() * 3);
    }

    void update()
    {
        if (buffer_->size() != data_.size())
        {
            buffer_->reserve(data_.size());
        }
        buffer_->setData(data_);
    }

    void release()
    {
        if (buffer_ != nullptr)
        {
            buffer_->release();
        }
        data_.swap(std::vector<float>{});
    }
};

class AttributeIndexBuffer
{
    std::vector<unsigned int> data_;
    std::shared_ptr<ElementArrayBuffer> buffer_;
    size_t dim_ = 3;

  public:
    static std::shared_ptr<AttributeIndexBuffer> create(size_t dim)
    {
        auto attr_buf = std::make_shared<AttributeIndexBuffer>();
        attr_buf->dim_ = dim;
        attr_buf->buffer_ = ElementArrayBuffer::create(1);
        return attr_buf;
    }

    const unsigned int *ptr() const
    {
        return &data_[0];
    }

    const glm::uvec3 *ptrUVec3() const
    {
        if (dim_ != 3)
        {
            throw std::runtime_error("Attempting to interpret " + std::to_string(dim_) +
                                     "D data as 3D");
        }
        return reinterpret_cast<const glm::uvec3 *>(&data_[0]);
    }

    const glm::uvec2 *ptrUVec2() const
    {
        if (dim_ != 2)
        {
            throw std::runtime_error("Attempting to interpret " + std::to_string(dim_) +
                                     "D data as 2D");
        }
        return reinterpret_cast<const glm::uvec2 *>(&data_[0]);
    }

    size_t size() const
    {
        return data_.size();
    }

    size_t count() const
    {
        return data_.size() / dim();
    }

    size_t dim() const
    {
        return dim_;
    }

    std::shared_ptr<ElementArrayBuffer> buffer() const
    {
        return buffer_;
    }

    const std::vector<unsigned int> &data() const
    {
        return data_;
    }

    std::vector<unsigned int> &data()
    {
        return data_;
    }

    void setData(const std::vector<unsigned int> &data)
    {
        data_ = data;
    }

    void setData(const std::vector<glm::uvec2> &data)
    {
        if (dim_ != 2)
        {
            throw std::runtime_error("Attempting to set 2D data, expected " + std::to_string(dim_) +
                                     "D data");
        }
        if (data.empty())
        {
            data_.clear();
        }
        else
        {
            data_.assign(glm::value_ptr(data[0]), glm::value_ptr(data[0]) + data.size() * 2);
        }
    }

    void setData(const std::vector<glm::uvec3> &data)
    {
        if (dim_ != 3)
        {
            throw std::runtime_error("Attempting to set 3D data, expected " + std::to_string(dim_) +
                                     "D data");
        }
        if (data.empty())
        {
            data_.clear();
        }
        else
        {
            data_.assign(glm::value_ptr(data[0]), glm::value_ptr(data[0]) + data.size() * 3);
        }
    }

    void update()
    {
        if (buffer_->size() != data_.size())
        {
            buffer_->reserve(data_.size());
        }
        buffer_->setData(data_);
    }

    void release()
    {
        if (buffer_ != nullptr)
        {
            buffer_->release();
        }
        data_.swap(std::vector<unsigned int>{});
    }
};

} // namespace rcube