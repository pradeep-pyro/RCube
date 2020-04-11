#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace rcube
{

const std::string ERROR_MESH_UNINITIALIZED = "Cannot use Mesh without initializing";
const std::string ERROR_MESH_PRIMITIVE_INDICES_MISMATCH =
    "Mismatch between mesh indices count and primitive";

//-----------------------------------------------------------------------------
// MeshData
//-----------------------------------------------------------------------------
void MeshData::clear()
{
    vertices.clear();
    normals.clear();
    colors.clear();
    texcoords.clear();
    indices.clear();
}

void MeshData::append(MeshData &other)
{
    assert(primitive == other.primitive && indexed == other.indexed);
    size_t offset = vertices.size();
    vertices.insert(vertices.end(), other.vertices.begin(), other.vertices.end());
    normals.insert(normals.end(), other.normals.begin(), other.normals.end());
    colors.insert(colors.end(), other.colors.begin(), other.colors.end());
    texcoords.insert(texcoords.end(), other.texcoords.begin(), other.texcoords.end());
    auto tmp = other.indices;
    for (auto &val : tmp)
    {
        val += static_cast<unsigned int>(offset);
    }
    indices.insert(indices.end(), tmp.begin(), tmp.end());
}

void MeshData::scaleAndCenter()
{
    glm::vec3 min = vertices[0];
    glm::vec3 max = vertices[0];
    for (const glm::vec3 &v : vertices)
    {
        if (v.x < min.x)
        {
            min.x = v.x;
        }
        if (v.y < min.y)
        {
            min.y = v.y;
        }
        if (v.z < min.z)
        {
            min.z = v.z;
        }
        if (v.x > max.x)
        {
            max.x = v.x;
        }
        if (v.y > max.y)
        {
            max.y = v.y;
        }
        if (v.z > max.z)
        {
            max.z = v.z;
        }
    }
    const glm::vec3 centroid = 0.5f * (max + min);
    const glm::vec3 size = glm::abs(max - min);
    const float scale = std::max(size.x, std::max(size.y, size.z));
    for (glm::vec3 &v : vertices)
    {
        v = 2.0f * (v - centroid) / scale;
    }
}

bool MeshData::valid() const
{
    if (vertices.empty())
    {
        return false;
    }
    if ((!colors.empty()) && (colors.size() != vertices.size()))
    {
        return false;
    }
    if ((!normals.empty()) && (normals.size() != vertices.size()))
    {
        return false;
    }
    if ((!texcoords.empty()) && (texcoords.size() != vertices.size()))
    {
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
// Mesh
//-----------------------------------------------------------------------------

std::shared_ptr<Mesh> Mesh::create(std::vector<std::shared_ptr<AttributeBuffer>> attributes, MeshPrimitive prim)
{
    auto mesh = std::make_shared<Mesh>();
    glGenVertexArrays(1, &mesh->vao_);
    glBindVertexArray(mesh->vao_);
    for (auto attr : attributes)
    {
        auto attrbuf = AttributeBuffer::create(attr->name(), attr->location(), attr->dim());
        attrbuf->buffer()->use();
        GLuint loc = static_cast<GLuint>(attrbuf->location());
        glVertexAttribPointer(loc, static_cast<GLint>(attr->dim()), GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(loc);
        mesh->attributes_[attr->name()] = attrbuf;
    }
    mesh->indices_ = AttributeIndexBuffer::create(prim == MeshPrimitive::Points ? 1 : (prim == MeshPrimitive::Lines ? 2 : 3));
    mesh->indices_->buffer()->use();
    mesh->init_ = true;
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGLError();
    return mesh;
}

std::shared_ptr<Mesh> Mesh::create(MeshPrimitive prim)
{
    return Mesh::create(
        {AttributeBuffer::create("positions", GLuint(AttributeLocation::POSITION), 3),
         AttributeBuffer::create("normals", GLuint(AttributeLocation::NORMAL), 3),
         AttributeBuffer::create("uvs", GLuint(AttributeLocation::UV), 2),
         AttributeBuffer::create("colors", GLuint(AttributeLocation::COLOR), 3),
         AttributeBuffer::create("tangents", GLuint(AttributeLocation::TANGENT), 3)}, prim);
}

void Mesh::release()
{
    if (!init_)
    {
        return;
    }
    for (auto &kv : attributes_)
    {
        kv.second->release();
    }
    if (vao_ != 0)
    {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    init_ = false;
}

GLuint Mesh::vao() const
{
    return vao_;
}

bool Mesh::valid() const
{
    return init_;
}

void Mesh::enableAttribute(std::string name)
{
    GLuint location = attributes_.at(name)->location();
    use();
    glEnableVertexAttribArray(location);
    done();
}

void Mesh::disableAttribute(std::string name)
{
    auto &attr = attributes_.at(name);
    checkGLError();
    use();
    GLuint id = attr->buffer()->id();
    GLuint location = attr->location();
    glDisableVertexAttribArray(location);
    if (attr->dim() == 3)
    {
        setDefaultValue(location, glm::vec3(1, 1, 1));
    }
    else if (attr->dim() == 2)
    {
        setDefaultValue(location, glm::vec2(0, 0));
    }
    checkGLError();
    done();
}

void Mesh::use() const
{
    if (!valid())
    {
        throw std::runtime_error(ERROR_MESH_UNINITIALIZED);
    }
    glBindVertexArray(vao_);
}

void Mesh::done() const
{
    glBindVertexArray(0);
}

size_t Mesh::numVertexData() const
{
    return attributes_.at("positions")->size();
}

size_t Mesh::numIndexData() const
{
    if (indices_ != nullptr)
    {
        return indices_->size();
    }
    return 0;
}

bool Mesh::hasAttribute(std::string name) const
{
    return attributes_.find(name) != attributes_.end();
}

void Mesh::uploadToGPU()
{
    done();
    attributes_["positions"]->setData(data.vertices);
    attributes_["normals"]->setData(data.normals);
    attributes_["colors"]->setData(data.colors);
    attributes_["uvs"]->setData(data.texcoords);
    attributes_["tangents"]->setData(data.tangents);
    if (data.indexed)
    {
        indices_->setData(data.indices);
        indices_->update();
    }
    for (auto &kv : attributes_)
    {
        if (kv.second->data().size() / kv.second->dim() != attributes_["positions"]->data().size() / attributes_["positions"]->dim())
        {
            disableAttribute(kv.first);
        }
        else
        {
            enableAttribute(kv.first);
            kv.second->update();
        }
    }
}

void Mesh::setDefaultValue(GLuint id, const glm::vec3 &val)
{
    glVertexAttrib3f(id, val[0], val[1], val[2]);
}
void Mesh::setDefaultValue(GLuint id, const glm::vec2 &val)
{
    glVertexAttrib2f(id, val[0], val[1]);
}

void Mesh::updateBVH()
{
    // TODO(pradeep): find a way to avoid creating all these primitives and reuse original mesh data
    std::vector<PrimitivePtr> prims;
    if (numIndexData() > 0)
    {
        prims.reserve(data.indices.size() / 3);
        size_t face_id = 0;
        for (size_t i = 0; i < data.indices.size(); i += 3)
        {
            prims.push_back(std::make_shared<Triangle>(
                face_id++, data.vertices[data.indices[i + 0]], data.vertices[data.indices[i + 1]],
                data.vertices[data.indices[i + 2]]));
        }
    }
    else
    {
        prims.reserve(data.vertices.size() / 3);
        size_t face_id = 0;
        for (size_t i = 0; i < data.vertices.size(); i += 3)
        {
            prims.push_back(std::make_shared<Triangle>(face_id++, data.vertices[i + 0],
                                                       data.vertices[i + 1], data.vertices[i + 2]));
        }
    }
    bvh_ = buildBVH(prims);
}

bool Mesh::rayIntersect(const Ray &ray, glm::vec3 &pt, size_t &id)
{
    if (bvh_ == nullptr)
    {
        return false;
    }
    PrimitivePtr prim;
    bool hit = bvh_->rayIntersect(ray, pt, prim);
    if (!hit)
    {
        return false;
    }
    id = prim->id();
    return true;
}

} // namespace rcube
