#include "diffuseprefilter.h"
#include "../meshgen/box.h"
#include "../render/checkglerror.h"

namespace rcube {

std::string DiffuseIrradianceShader::vertexShader() {
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

std::string DiffuseIrradianceShader::fragmentShader() {
    return
R"(
#version 420

#define IMPORTANCE_SAMPLING

out vec4 frag_color;
in vec3 direction;

#ifdef IMPORTANCE_SAMPLING
uniform int num_samples;
#else
uniform float sample_spacing = 0.002;
#endif

layout (binding = 3) uniform samplerCube env_map;

const float PI = 3.14159265358979323846;

uint reverseBits32(uint n) {
    n = (n << 16) | (n >> 16);  // Swap first and last 16 bits
    n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);  // Swap consecutive 8 bits in the first half & second half
    n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);  // Continue similarly
    n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
    n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
    return n;
}

float radicalInverse(uint n) {
    n = reverseBits32(n);
    // Divide by 2^32
    return float(n) * 2.3283064365386963e-10;
}

// Generate low-discrepancy sequence using the Hammersley method
vec2 hammersleySample(int i, int N) {
    return vec2(float(i) / float(N), radicalInverse(i));
}

// Transforms uniform sample u into a cosine weighted sample
vec2 cosineSample(vec2 u) {
    return vec2(asin(sqrt(u[0])), 2 * PI * u[1]);
}

vec3 cosineSampleCartesian(vec2 u) {
    float sqrt_u0 = sqrt(u[0]);
    // sample is vec2(asin(sqrt_u0), 2 * PI * u[1]);
    // x = sin u0 cos u1
    float x = sqrt_u0 * cos(2 * PI * u[1]);
    // y = sin u0 sin u1
    float y = sqrt_u0 * sin(2 * PI * u[1]);
    // z = cos u0, and cos(asin(x)) = sqrt(1 - x^2)
    float z = sqrt(max(0, 1 - u[0]));
    return vec3(x, y, z);
}

void main() {
    vec3 N = normalize(direction);
    // Tangent space calculation coordinate system
    vec3 up = vec3(0, 1, 0);
    vec3 right = cross(up, N);
    up = cross(N, right);

    int count = 0;
    vec3 irradiance = vec3(0.0);

#ifdef IMPORTANCE_SAMPLING
    for (int i = 0; i < num_samples; ++i) {
        vec2 u = hammersleySample(i, num_samples);
        vec3 tangent_sample = cosineSampleCartesian(u);
        vec3 radiance = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * N;
        irradiance += texture(env_map, radiance).rgb;
        count++;
    }
#else
    for(float phi = 0.0; phi < 2.0 * PI; phi += sample_spacing) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sample_spacing) {
            vec3 tangent_sample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            vec3 radiance = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * N;
            irradiance += texture(env_map, radiance).rgb * cos(theta) * sin(theta);
            count++;
        }
    }
    irradiance *= PI;
#endif
    irradiance *= (1.0 / float(count));
    frag_color = vec4(irradiance, 1.0);
}

)";
}

std::string DiffuseIrradianceShader::geometryShader() {
    return "";
}

void DiffuseIrradianceShader::setUniforms() {
    if (environment_map != nullptr) {
        environment_map->use(3);
    }
    shader_->setUniform("num_samples", num_samples);
}

int DiffuseIrradianceShader::renderPriority() const {
    return RenderPriority::Opaque;
}

///////////////////////////////////////////////////////////////////////////////

DiffusePrefilter::DiffusePrefilter(unsigned int resolution)
        : resolution_(resolution){
    // Create a unit cube in clip space
    cube_ = Mesh::create();
    cube_->data = box(2.0, 2.0, 2.0, 1, 1, 1);
    cube_->uploadToGPU();

    // Compile the irradiance shader
    shader_.initialize();

    // Create framebuffer to hold result
    fbo_ = Framebuffer::create(resolution, resolution);
    fbo_->addColorAttachment(TextureInternalFormat::RGB16F);
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
    shader_.num_samples = num_samples;
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
