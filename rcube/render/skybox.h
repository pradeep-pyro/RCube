#ifndef SKYBOX_H
#define SKYBOX_H

#include "Texture.h"
#include "ShaderProgram.h"
#include "Image.h"
#include <vector>

class Skybox {
public:
    Skybox();
    void free();
    void render();
    std::shared_ptr<TextureCube> texture;
private:
    ShaderProgram shader_;
    GLuint vao_, vbo_;
};

#endif // SKYBOX_H
