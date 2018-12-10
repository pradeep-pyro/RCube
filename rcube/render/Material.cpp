#include "Material.h"

Material::Material() : depth_test(true), depth_mask(true),
    shader_(std::make_shared<ShaderProgram>()), init_(false) {
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
    // To minimize program binding change accidentally
    shader_->use();
    // Cache previous state
    //glGetBooleanv(GL_DEPTH_WRITEMASK, &prev_state_.depthmask);
    //glGetBooleanv(GL_DEPTH_TEST, &prev_state_.depthtest);
    if (depth_test) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(depth_mask);

    // Bind shader and set uniforms
    setUniforms();
}

void Material::done() {
    shader_->done();
    /*
    // Restore previous state
    if (prev_state_.depthtest) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(prev_state_.depthmask);
    */
}
