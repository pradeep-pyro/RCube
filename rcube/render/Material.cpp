#include "Material.h"

Material::Material() : shader_(std::make_shared<ShaderProgram>()), init_(false) {
}

Material::~Material() {
}

void Material::initialize() {
    if (init_) {
        return;
    }
    std::string vert_src = vertexShader();
    std::string frag_src = fragmentShader();
    std::string geom_src = geometryShader();
    if (vert_src.size() > 0) {
        if (!shader_->setVertexShader(vert_src, true)) {
            throw std::runtime_error("Unable to compile vertex shader");
        }
    }
    if (frag_src.size() > 0) {
        if (!shader_->setFragmentShader(frag_src, true)) {
            throw std::runtime_error("Unable to compile fragment shader");
        }
    }
    if (geom_src.size() > 0) {
        if (!shader_->setGeometryShader(geom_src, true)) {
            throw std::runtime_error("Unable to compile geometry shader");
        }
    }
    if (!shader_->link(true)) {
        throw std::runtime_error("Unable to link shader");
    }
}

std::shared_ptr<ShaderProgram> Material::shader() const {
    return shader_;
}

void Material::use() {
    shader_->use();
    setUniforms();
}

void Material::done() {
    shader_->done();
}
