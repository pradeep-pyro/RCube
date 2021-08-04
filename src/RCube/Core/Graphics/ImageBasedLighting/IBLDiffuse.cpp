#include "RCube/Core/Graphics/ImageBasedLighting/IBLDiffuse.h"
#include "RCube/Core/Graphics/MeshGen/Box.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"

namespace rcube
{

const static std::string DiffuseIrradianceVertexShader = R"(
#version 420
layout (location = 0) in vec3 position;

out vec3 direction;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
    direction = position;
    gl_Position =  projection_matrix * mat4(mat3(view_matrix)) * vec4(position, 1);
}
)";

const static std::string DiffuseIrradianceFragmentShader =
    R"(
#version 420

out vec4 frag_color;
in vec3 direction;

uniform int num_samples;

uniform samplerCube env_map;

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
vec2 hammersleySample(uint i, int N) {
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

    vec3 irradiance = vec3(0.0);

    for (int i = 0; i < num_samples; ++i) {
        vec2 u = hammersleySample(i, num_samples);
        vec3 tangent_sample = cosineSampleCartesian(u);
        vec3 radiance = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * N;
        irradiance += texture(env_map, radiance).rgb;
    }
    irradiance *= (1.0 / float(num_samples));
    frag_color = vec4(irradiance, 1.0);
}
)";

///////////////////////////////////////////////////////////////////////////////

IBLDiffuse::IBLDiffuse(unsigned int resolution, int num_samples)
    : resolution_(resolution), num_samples_(num_samples)
{
    // Create a unit cube in clip space
    cube_ = Mesh::create(box(2.0, 2.0, 2.0, 1, 1, 1));
    cube_->uploadToGPU();

    // Compile the irradiance shader
    shader_ =
        ShaderProgram::create(DiffuseIrradianceVertexShader, DiffuseIrradianceFragmentShader, true);

    // Create framebuffer to hold result
    fbo_ = Framebuffer::create();
    fbo_->setColorAttachment(
        0, Texture2D::create(resolution, resolution, 1, TextureInternalFormat::RGB16F));
    fbo_->setDepthAttachment(
        Texture2D::create(resolution, resolution, 1, TextureInternalFormat::Depth32));

    // Matrices for rendering the cubemap from cameras set pointing at the
    // cube faces
    projection_ = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
    views_ = {glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
              glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
              glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
              glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
              glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
              glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))};

    // Create renderer
    rdr_.initialize();
}

IBLDiffuse::~IBLDiffuse()
{
    cube_->release();
    rdr_.cleanup();
    fbo_->release();
    shader_->release();
}

int IBLDiffuse::numSamples() const
{
    return num_samples_;
}

void IBLDiffuse::setNumSamples(int num_samples)
{
    num_samples_ = num_samples;
}

std::shared_ptr<TextureCubemap> IBLDiffuse::irradiance(std::shared_ptr<TextureCubemap> env_map)
{
    auto irradiance_map =
        TextureCubemap::create(resolution_, resolution_, 1, true, TextureInternalFormat::RGB16F);

    glm::mat4 eye(1.0);
    glm::vec3 eye_pos(0., 0., 0.);
    RenderTarget rt;
    rt.framebuffer = fbo_->id();
    rt.clear_color = {glm::vec4(0.f)};
    rt.clear_depth_buffer = true;
    rt.clear_stencil_buffer = false;
    rt.viewport_origin = glm::ivec2(0);
    rt.viewport_size = glm::ivec2(resolution_);
    RenderSettings s;
    DrawCall dc;
    dc.cubemaps.push_back({env_map->id(), 0});
    dc.mesh = GLRenderer::getDrawCallMeshInfo(cube_);
    dc.shader = shader_;
    for (unsigned int i = 0; i < 6; ++i)
    {
        dc.update_uniforms = [&](std::shared_ptr<ShaderProgram> shader) {
            shader->uniform("num_samples").set(num_samples_);
            shader->uniform("view_matrix").set(views_[i]);
            shader->uniform("projection_matrix").set(projection_);
        };

        rdr_.draw(rt, s, {dc});
        fbo_->copySubImage(0, irradiance_map, TextureCubemap::Side(i), 0, glm::ivec2(0),
                           glm::ivec2(resolution_));
    }

    return irradiance_map;
}

} // namespace rcube
