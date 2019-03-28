#include <iostream>
#include "../prefilter/diffuseprefilter.h"
#include "../meshgen/box.h"
#include "../render/checkglerror.h"

namespace rcube {

std::string IrradianceShader::vertexShader() {
    return
R"(
#version 420
layout (location = 0) in vec3 position;

out vec3 direction;

layout (std140, binding=0) uniform Matrices {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
};

void main() {
    direction = position;
    gl_Position =  projection_matrix * view_matrix * vec4(position, 1);
}
)";
}

std::string IrradianceShader::fragmentShader() {
    return
R"(
#version 420
out vec4 frag_color;
in vec3 direction;

layout (binding = 3) uniform samplerCube env_map;

const float PI = 3.14159265359;

void main() {
    vec3 N = normalize(direction);
    // tangent space calculation from origin point
    vec3 up = vec3(0, 1, 0);
    vec3 right = cross(up, N);
    up = cross(N, right);

    float sampleDelta = 0.025;
    int count = 0;

    vec3 irradiance = vec3(0.0);
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            vec3 tangent_sample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            vec3 radiance = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * N;
            irradiance += texture(env_map, radiance).rgb * cos(theta) * sin(theta);
            count++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(count));
    frag_color = vec4(irradiance, 1.0);
}

)";
}

std::string IrradianceShader::geometryShader() {
    return "";
}

void IrradianceShader::setUniforms() {
    if (environment_map != nullptr) {
        environment_map->use(3);
    }
}

int IrradianceShader::renderPriority() const {
    return RenderPriority::Opaque;
}

///////////////////////////////////////////////////////////////////////////////

DiffusePrefilter::DiffusePrefilter(unsigned int resolution) {
    // Create a unit cube in clip space
    cube_ = Mesh::create();
    cube_->data = box(2.0, 2.0, 2.0, 1, 1, 1);
    cube_->uploadToGPU();

    // Compile the irradiance shader
    shader_.initialize();

    // Create framebuffer to hold result
    fbo_ = Framebuffer::create(resolution, resolution);
    fbo_->addColorAttachment(TextureInternalFormat::RGB8);
    fbo_->addDepthAttachment(TextureInternalFormat::Depth24Stencil8);

    // Matrices for rendering the cubemap from cameras set pointing at the
    // cube faces
    projection_ = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
    views_ = {
        glm::lookAt(glm::vec3(0, 0, 0), glm::vec3( 1,  0,  0), glm::vec3(0, -1,  0)),
        glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1,  0,  0), glm::vec3(0, -1,  0)),
        glm::lookAt(glm::vec3(0, 0, 0), glm::vec3( 0,  1,  0), glm::vec3(0,  0,  1)),
        glm::lookAt(glm::vec3(0, 0, 0), glm::vec3( 0, -1,  0), glm::vec3(0,  0, -1)),
        glm::lookAt(glm::vec3(0, 0, 0), glm::vec3( 0,  0,  1), glm::vec3(0, -1,  0)),
        glm::lookAt(glm::vec3(0, 0, 0), glm::vec3( 0,  0, -1), glm::vec3(0, -1,  0))
    };

    // Create renderer
    rdr_.initialize();
}

DiffusePrefilter::~DiffusePrefilter() {
    cube_->release();
    rdr_.cleanup();
    fbo_->release();
}

std::shared_ptr<TextureCubemap>
DiffusePrefilter::prefilter(std::shared_ptr<TextureCubemap> env_map) {
    auto irradiance_map = TextureCubemap::create(resolution_, resolution_, 1,
                                                 true, TextureInternalFormat::RGB16F);
    shader_.environment_map = env_map;
    rdr_.resize(0, 0, resolution_, resolution_);

    glm::mat4 eye(1.0);
    fbo_->use();
    for (unsigned int i = 0; i < 6; ++i) {
        rdr_.clear();
        rdr_.setCamera(views_[i], projection_, eye);
        rdr_.render(cube_.get(), &shader_, eye);
        irradiance_map->use();
        glCopyTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, 0, 0,
                            fbo_->width(), fbo_->height());
    }
    fbo_->done();

    return irradiance_map;
}

} // namespace rcube
