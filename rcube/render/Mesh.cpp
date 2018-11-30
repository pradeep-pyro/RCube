#include <stdexcept>
#include "Mesh.h"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
using namespace std;

//-----------------------------------------------------------------------------
// MeshData
//-----------------------------------------------------------------------------
void MeshData::clear() {
    vertices.clear();
    normals.clear();
    colors.clear();
    texcoords.clear();
    indices.clear();
}

void MeshData::append(MeshData &other) {
    assert(primitive == other.primitive && indexed == other.indexed);
    size_t offset = vertices.size();
    vertices.insert(vertices.end(), other.vertices.begin(), other.vertices.end());
    normals.insert(normals.end(), other.normals.begin(), other.normals.end());
    colors.insert(colors.end(), other.colors.begin(), other.colors.end());
    texcoords.insert(texcoords.end(), other.texcoords.begin(), other.texcoords.end());
    auto tmp = other.indices;
    for (auto &val : tmp) {
        val += offset;
    }
    indices.insert(indices.end(), tmp.begin(), tmp.end());
}

//-----------------------------------------------------------------------------
// Mesh
//-----------------------------------------------------------------------------

Mesh::Mesh() : num_vertices_(0), num_primitives_(0), indexed_(false), primitive_(MeshPrimitive::Triangles) {
    // Generate vao and vertex vbo
    glGenVertexArrays(1, &glbuf_.vao);
    glBindVertexArray(glbuf_.vao);
    glGenBuffers(1, &glbuf_.vertices);
    glBindBuffer(GL_ARRAY_BUFFER, glbuf_.vertices);
    GLuint vid = static_cast<GLuint>(MeshAttributes::Vertices);
    glEnableVertexAttribArray(vid);
    glVertexAttribPointer(vid, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    disableAttribute(MeshAttributes::Colors);
    disableAttribute(MeshAttributes::Normals);
    disableAttribute(MeshAttributes::TexCoords);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Mesh::~Mesh() {
}

void Mesh::release() {
    if (glbuf_.vao != 0) {
        glDeleteVertexArrays(1, &glbuf_.vao);
        glbuf_.vao = 0;
    }
    if (glbuf_.vertices != 0) {
        glDeleteBuffers(1, &glbuf_.vertices);
        glbuf_.vertices = 0;
    }
    if (glbuf_.normals != 0) {
        glDeleteBuffers(1, &glbuf_.normals);
        glbuf_.normals = 0;
    }
    if (glbuf_.indices != 0) {
        glDeleteBuffers(1, &glbuf_.indices);
        glbuf_.indices = 0;
    }
    if (glbuf_.texcoords != 0) {
        glDeleteBuffers(1, &glbuf_.texcoords);
        glbuf_.texcoords = 0;
    }
    if (glbuf_.colors != 0) {
        glDeleteBuffers(1, &glbuf_.colors);
        glbuf_.colors = 0;
    }
}

GLuint Mesh::vao() const {
    return glbuf_.vao;
}

void Mesh::enableAttribute(MeshAttributes attr) {
    GLuint *id;
    int dim = 3;
    if (attr == MeshAttributes::Normals) {
        id = &glbuf_.normals;
    }
    else if (attr == MeshAttributes::Colors) {
        id = &glbuf_.colors;
    }
    else if (attr == MeshAttributes::Vertices) {
        id = &glbuf_.vertices;
    }
    else {
        id = &glbuf_.texcoords;
        dim = 2;
    }

    GLuint gl_attr = static_cast<GLuint>(attr);
    use();
    if (*id == 0) {
        glGenBuffers(1, id);
        glBindBuffer(GL_ARRAY_BUFFER, *id);
        glVertexAttribPointer(gl_attr, dim, GL_FLOAT, GL_FALSE, 0, NULL);
    }
    glEnableVertexArrayAttrib(glbuf_.vao, gl_attr);
    done();
}

void Mesh::disableAttribute(MeshAttributes attr) {
    use();
    GLuint gl_attr = static_cast<GLuint>(attr);
    glDisableVertexAttribArray(gl_attr);
    if (attr == MeshAttributes::Normals) {
        setDefaultValue(gl_attr, glm::vec3(0, 0, 1));
    }
    else if (attr == MeshAttributes::Colors) {
        setDefaultValue(gl_attr, glm::vec3(1));
    }
    else if (attr == MeshAttributes::TexCoords) {
        setDefaultValue(gl_attr, glm::vec2(0));
    }
    done();
}

void Mesh::use() const {
    glBindVertexArray(glbuf_.vao);
}

void Mesh::done() const {
    glBindVertexArray(0);
}

size_t Mesh::numVertices() const {
    return num_vertices_;
}

size_t Mesh::numPrimitives() const {
    return num_primitives_;
}

void Mesh::setVertices(const std::vector<glm::vec3> &data) {
    enableAttribute(MeshAttributes::Vertices);
    setArrayBuffer(glbuf_.vertices, glm::value_ptr(data[0]), 3 * data.size());
    num_vertices_ = data.size();
}

void Mesh::setNormals(const std::vector<glm::vec3> &data) {
    enableAttribute(MeshAttributes::Normals);
    setArrayBuffer(glbuf_.normals, glm::value_ptr(data[0]), 3 * data.size());
}

void Mesh::setTextureCoords(const std::vector<glm::vec2> &data) {
    enableAttribute(MeshAttributes::TexCoords);
    setArrayBuffer(glbuf_.texcoords, glm::value_ptr(data[0]), 2 * data.size());
}

void Mesh::setColors(const std::vector<glm::vec3> &data) {
    enableAttribute(MeshAttributes::Colors);
    setArrayBuffer(glbuf_.colors, glm::value_ptr(data[0]), 3 * data.size());
}

void Mesh::setIndices(const std::vector<unsigned int> &data) {
    int dim = 3;
    if (primitive_ == MeshPrimitive::Points) {
        dim = 1;
    }
    else if (primitive_ == MeshPrimitive::Lines) {
        dim = 2;
    }
    assert (data.size() % dim == 0);
    setIndexed(true);
    setElementBuffer(data.data(), data.size());
    num_primitives_ = data.size();
}

void Mesh::setMeshData(const MeshData &data) {
    setVertices(data.vertices);
    if (data.normals.size() > 0) {
        setNormals(data.normals);
    }
    if (data.texcoords.size() > 0) {
        setTextureCoords(data.texcoords);
    }
    if (data.colors.size() > 0) {
        setColors(data.colors);
    }
    if (data.indexed && data.indices.size() > 0) {
        setIndexed(true);
        setIndices(data.indices);
    }
    setPrimitive(data.primitive);
}

bool Mesh::indexed() const {
    return indexed_;
}
void Mesh::setIndexed(bool flag) {
    indexed_ = flag;
    if (indexed_ && glbuf_.indices == 0) {
        use();
        glGenBuffers(1, &glbuf_.indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glbuf_.indices);
        done();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

MeshPrimitive Mesh::primitive() const {
    return primitive_;
}

void Mesh::setPrimitive(MeshPrimitive prim) {
    primitive_ = prim;
}

// Private methods

void Mesh::setArrayBuffer(GLuint id, const float *data, unsigned int count) {
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void Mesh::setElementBuffer(const unsigned int *data, unsigned int count) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glbuf_.indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * count, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void Mesh::setDefaultValue(GLuint id, const glm::vec3 &val) {
    glVertexAttrib3f(id, val[0], val[1], val[2]);
}
void Mesh::setDefaultValue(GLuint id, const glm::vec2 &val) {
    glVertexAttrib2f(id, val[0], val[1]);
}
