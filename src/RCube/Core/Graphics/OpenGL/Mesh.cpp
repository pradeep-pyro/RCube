#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
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
        val += offset;
    }
    indices.insert(indices.end(), tmp.begin(), tmp.end());
}

bool MeshData::valid() const
{
    return vertices.size() == colors.size() == normals.size() == texcoords.size();
}

//-----------------------------------------------------------------------------
// Mesh
//-----------------------------------------------------------------------------

std::shared_ptr<Mesh> Mesh::create()
{
    auto mesh = std::make_shared<Mesh>();
    glGenVertexArrays(1, &mesh->glbuf_.vao);
    glBindVertexArray(mesh->glbuf_.vao);
    glGenBuffers(1, &mesh->glbuf_.vertices);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->glbuf_.vertices);
    GLuint vid = static_cast<GLuint>(MeshAttributes::Vertices);
    glEnableVertexAttribArray(vid);
    glVertexAttribPointer(vid, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    mesh->init_ = true;
    mesh->disableAttribute(MeshAttributes::Colors);
    mesh->disableAttribute(MeshAttributes::Normals);
    mesh->disableAttribute(MeshAttributes::TexCoords);
    mesh->disableAttribute(MeshAttributes::Tangents);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGLError();
    return mesh;
}

void Mesh::release()
{
    if (!init_)
    {
        return;
    }
    if (glbuf_.vao != 0)
    {
        glDeleteVertexArrays(1, &glbuf_.vao);
        glbuf_.vao = 0;
    }
    if (glbuf_.vertices != 0)
    {
        glDeleteBuffers(1, &glbuf_.vertices);
        glbuf_.vertices = 0;
    }
    if (glbuf_.normals != 0)
    {
        glDeleteBuffers(1, &glbuf_.normals);
        glbuf_.normals = 0;
    }
    if (glbuf_.indices != 0)
    {
        glDeleteBuffers(1, &glbuf_.indices);
        glbuf_.indices = 0;
    }
    if (glbuf_.texcoords != 0)
    {
        glDeleteBuffers(1, &glbuf_.texcoords);
        glbuf_.texcoords = 0;
    }
    if (glbuf_.colors != 0)
    {
        glDeleteBuffers(1, &glbuf_.colors);
        glbuf_.colors = 0;
    }
    if (glbuf_.tangents != 0)
    {
        glDeleteBuffers(1, &glbuf_.tangents);
        glbuf_.colors = 0;
    }
    init_ = false;
}

GLuint Mesh::vao() const
{
    return glbuf_.vao;
}

bool Mesh::valid() const
{
    return init_;
}

bool Mesh::indexed() const
{
    return indexed_;
}

void Mesh::enableAttribute(MeshAttributes attr)
{
    GLuint *id;
    int dim = 3;
    if (attr == MeshAttributes::Normals)
    {
        id = &glbuf_.normals;
        has_normals_ = true;
    }
    else if (attr == MeshAttributes::Colors)
    {
        id = &glbuf_.colors;
        has_colors_ = true;
    }
    else if (attr == MeshAttributes::Vertices)
    {
        id = &glbuf_.vertices;
        has_vertices_ = true;
    }
    else if (attr == MeshAttributes::Tangents)
    {
        id = &glbuf_.tangents;
        has_tangents_ = true;
    }
    else if (attr == MeshAttributes::TexCoords)
    {
        id = &glbuf_.texcoords;
        dim = 2;
        has_texcoords_ = true;
    }

    GLuint gl_attr = static_cast<GLuint>(attr);
    use();
    if (*id == 0)
    {
        glGenBuffers(1, id);
        glBindBuffer(GL_ARRAY_BUFFER, *id);
        glVertexAttribPointer(gl_attr, dim, GL_FLOAT, GL_FALSE, 0, NULL);
    }
    checkGLError();
    glBindVertexArray(glbuf_.vao);
    glEnableVertexAttribArray(gl_attr);
    // glEnableVertexArrayAttrib(glbuf_.vao, gl_attr);
    done();
}

void Mesh::disableAttribute(MeshAttributes attr)
{
    checkGLError();
    use();
    GLuint gl_attr = static_cast<GLuint>(attr);
    glDisableVertexAttribArray(gl_attr);
    if (attr == MeshAttributes::Normals)
    {
        setDefaultValue(gl_attr, glm::vec3(0, 0, 1));
        has_normals_ = false;
    }
    else if (attr == MeshAttributes::Colors)
    {
        setDefaultValue(gl_attr, glm::vec3(1));
        has_colors_ = false;
    }
    else if (attr == MeshAttributes::TexCoords)
    {
        setDefaultValue(gl_attr, glm::vec2(0));
        has_texcoords_ = false;
    }
    else if (attr == MeshAttributes::Tangents)
    {
        setDefaultValue(gl_attr, glm::vec3(1, 0, 0));
        has_tangents_ = false;
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
    glBindVertexArray(glbuf_.vao);
}

void Mesh::done() const
{
    glBindVertexArray(0);
}

void Mesh::setIndexed(bool flag)
{
    indexed_ = flag;
    if (flag && (glbuf_.indices == 0))
    {
        use();
        glGenBuffers(1, &glbuf_.indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glbuf_.indices);
        done();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    checkGLError();
}

size_t Mesh::numVertices() const
{
    return num_vertices_;
}

size_t Mesh::numPrimitives() const
{
    return num_primitives_;
}

bool Mesh::hasAttribute(MeshAttributes attr) const
{
    if (attr == MeshAttributes::Vertices)
    {
        return has_vertices_;
    }
    if (attr == MeshAttributes::Normals)
    {
        return has_normals_;
    }
    if (attr == MeshAttributes::Colors)
    {
        return has_colors_;
    }
    if (attr == MeshAttributes::TexCoords)
    {
        return has_texcoords_;
    }
    if (attr == MeshAttributes::Tangents)
    {
        return has_tangents_;
    }
}

void Mesh::addCustomAttribute(const std::string &name, GLuint attribute_location,
                              GLDataType attribute_type)
{
    assert(valid());
    size_t num_elements = data.vertices.size();
    int dim = 1;
    switch (attribute_type)
    {
    case GLDataType::Float:
        break;
    case GLDataType::Vec2f:
        num_elements *= 2;
        dim = 2;
        break;
    case GLDataType::Vec3f:
        num_elements *= 3;
        dim = 3;
        break;
    case GLDataType::Vec4f:
        num_elements *= 4;
        dim = 4;
        break;
    default:
        throw std::runtime_error(
            "Unsupported attribute type. Only 1D, 2D, 3D and 4D floats are supported for now.");
    }

    std::shared_ptr<Buffer> buf = Buffer::create(num_elements);
    buf->use();
    use();
    glVertexAttribPointer(attribute_location, dim, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(attribute_location);
    done();
    buf->done();
    custom_attributes_.push_back({name, attribute_location, attribute_type, buf});
}

Attribute &Mesh::customAttribute(const std::string &name)
{
    auto it = std::find_if(custom_attributes_.begin(), custom_attributes_.end(),
                           [&](Attribute &attr) { return attr.name == name; });
    if (it == custom_attributes_.end())
    {
        throw std::runtime_error("Attribute named " + name + " not found.");
    }
    return *it;
}

void Mesh::uploadToGPU(bool clear_cpu_data)
{
    setArrayBuffer(glbuf_.vertices, glm::value_ptr(data.vertices[0]), 3 * data.vertices.size());
    num_vertices_ = 3 * data.vertices.size();

    if (data.normals.size() > 0 && data.normals.size() == data.vertices.size())
    {
        enableAttribute(MeshAttributes::Normals);
        setArrayBuffer(glbuf_.normals, glm::value_ptr(data.normals[0]), 3 * data.normals.size());
    }
    else
    {
        checkGLError();
        disableAttribute(MeshAttributes::Normals);
    }

    if (data.texcoords.size() > 0 && data.texcoords.size() == data.vertices.size())
    {
        enableAttribute(MeshAttributes::TexCoords);
        setArrayBuffer(glbuf_.texcoords, glm::value_ptr(data.texcoords[0]),
                       2 * data.texcoords.size());
    }
    else
    {
        checkGLError();
        disableAttribute(MeshAttributes::TexCoords);
    }

    if (data.colors.size() > 0 && data.colors.size() == data.vertices.size())
    {
        enableAttribute(MeshAttributes::Colors);
        setArrayBuffer(glbuf_.colors, glm::value_ptr(data.colors[0]), 3 * data.colors.size());
    }
    else
    {
        checkGLError();
        disableAttribute(MeshAttributes::Colors);
    }

    if (data.tangents.size() > 0 && data.tangents.size() == data.vertices.size())
    {
        enableAttribute(MeshAttributes::Tangents);
        setArrayBuffer(glbuf_.tangents, glm::value_ptr(data.tangents[0]), 3 * data.tangents.size());
    }
    else
    {
        checkGLError();
        disableAttribute(MeshAttributes::Tangents);
    }

    if (data.indexed)
    {
        int dim = 3;
        if (data.primitive == MeshPrimitive::Points)
        {
            dim = 1;
        }
        else if (data.primitive == MeshPrimitive::Lines)
        {
            dim = 2;
        }
        if (data.indices.size() % dim != 0)
        {
            throw std::runtime_error(ERROR_MESH_PRIMITIVE_INDICES_MISMATCH);
        }
        setIndexed(true);
        setElementBuffer(data.indices.data(), data.indices.size());
        num_primitives_ = data.indices.size();
    }
    else
    {
        setIndexed(false);
    }

    checkGLError();
    if (clear_cpu_data)
    {
        data.clear();
    }
}

// Private methods

void Mesh::setArrayBuffer(GLuint id, const float *data, unsigned int count)
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void Mesh::setElementBuffer(const unsigned int *data, unsigned int count)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glbuf_.indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * count, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
    if (indexed())
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
