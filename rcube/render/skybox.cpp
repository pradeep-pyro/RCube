#include "skybox.h"
#include <string>

const float skybox_vertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

const std::string skybox_vs = R"(
#version 420

layout (location = 0) in vec3 position;
out vec3 texcoords;

layout (std140, binding=0) uniform Matrices {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
};

void main() {
    texcoords = position;
    vec4 pos = projection_matrix * mat4(mat3(view_matrix)) * vec4(position, 1.0);
    gl_Position = pos.xyww;
}
)";

const std::string skybox_fs = R"(
#version 420

out vec4 frag_color;
in vec3 texcoords;

layout(binding = 3) uniform samplerCube skybox;

void main() {
    frag_color = texture(skybox, texcoords);
}
)";

Skybox::Skybox() {
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);
    shader_.setVertexShader(skybox_vs);
    shader_.setFragmentShader(skybox_fs);
    shader_.link();
    texture = std::make_shared<TextureCube>();
}

void Skybox::free() {
    glDeleteBuffers(1, &vbo_);
    texture->free();
    shader_.release();
}

void Skybox::render() {
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    texture->use(3);
    shader_.use();
    glBindVertexArray(vao_);
    shader_.drawArrays(GL_TRIANGLES, 0, 36);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    shader_.done();
    texture->done();
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}
