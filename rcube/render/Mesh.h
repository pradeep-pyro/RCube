#ifndef MESH_H
#define MESH_H

#include <vector>
#include "glm/glm.hpp"
#include "glad/glad.h"
#include <memory>

enum class MeshPrimitive {
    Points = GL_POINTS, Lines = GL_LINES, Triangles = GL_TRIANGLES
};

enum class MeshAttributes {
    Vertices = 0,
    Normals = 1,
    TexCoords = 2,
    Colors = 3,
};

struct MeshData {
    MeshData() : primitive(MeshPrimitive::Triangles), indexed(false) {
    }
    std::vector<glm::vec3> vertices, normals, colors;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned int> indices;
    MeshPrimitive primitive;
    bool indexed;

    void clear();

    void append(MeshData &other);
};

// Represents a 3D triangle/line Mesh with vertex positions, normals,
// texcoords, colors using OpenGL buffers
class Mesh {
public:

    Mesh();

    ~Mesh();

    Mesh(const Mesh &other) = delete;

    Mesh & operator=(const Mesh &other) = delete;

    /**
     * Initialize actually creates the vertex attribute object and buffers on the OpenGL side
     */
    static std::shared_ptr<Mesh> create();

    void release();

    GLuint vao() const;

    bool valid() const;

    void use() const;

    void done() const;

    void enableAttribute(MeshAttributes attr);

    bool hasAttribute(MeshAttributes attr) const;

    void disableAttribute(MeshAttributes attr);

    void setVertices(const std::vector<glm::vec3> &data);

    void setNormals(const std::vector<glm::vec3> &data);

    void setTextureCoords(const std::vector<glm::vec2> &data);

    void setColors(const std::vector<glm::vec3> &data);

    void setIndices(const std::vector<unsigned int> &data);

    void setIndices(const std::vector<glm::uvec2> &data);

    void setIndices(const std::vector<glm::uvec3> &data);

    bool indexed() const;

    void setIndexed(bool flag);

    size_t numVertices() const;

    size_t numPrimitives() const;

    MeshPrimitive primitive() const;

    void setPrimitive(MeshPrimitive prim);

    void setMeshData(const MeshData &data);

private:

    void setArrayBuffer(GLuint id, const float *data, unsigned int count);

    void setElementBuffer(const unsigned int *data, unsigned int count);

    void setDefaultValue(GLuint id, const glm::vec3 &val);

    void setDefaultValue(GLuint id, const glm::vec2 &val);

    // Opengl Buffer IDs
    struct GLBufferIDs {
        GLBufferIDs() : vao(0), vertices(0), normals(0), indices(0),
            texcoords(0), colors(0) {
        }
        GLuint vao, vertices, normals, indices, texcoords, colors;
    };
    GLBufferIDs glbuf_;

    // Number of elements
    size_t num_vertices_ = 0;
    size_t num_primitives_ = 0;

    // Flags
    bool indexed_ = false;
    bool init_ = false;
    MeshPrimitive primitive_ = MeshPrimitive::Triangles;
};

#endif // GEOMETRY_H
